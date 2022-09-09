#!/usr/bin/env python3
import yaml
from yaml.loader import SafeLoader
import re
import sys

def binpack(bins, fields):
    if len(fields) == 0:
        return [[],[]]
    f, s = fields[0]
    for i, b in enumerate(bins):
        if s > b:
            continue
        new_bins = bins.copy()
        new_bins[i] -= s
        packing = binpack(new_bins, fields[1:])
        if packing:
            packing[i].append((f,s))
            return packing
    return None

def get_offset(b, f):
    offset = 0
    for (ff, s) in b:
        if ff == f:
            return offset
        offset += s * 8

def get_bin(bins, f):
    for i, b in enumerate(bins):
        for (ff,s) in b:
            if ff == f:
                return i, b

def make_assert(s):
    print(f"kassert({s});")

def make_getter(cap_name, name, size, bins):
    if name == 'padd':
        return
    print(f"static inline uint64_t cap_{cap_name}_get_{name}(cap_t cap) {{")
    make_assert(f"cap_is_type(cap, CAP_TYPE_{cap_name.upper()})")
    i, b = get_bin(bins, name)
    offset = get_offset(b, name)
    if i == 0:
        offset += 8
    if offset:
        print(f"return (cap.word{i} >> {offset}) & 0x{'ff'*size}ull;")
    else:
        print(f"return cap.word{i} & 0x{'ff'*size}ull;")
    print('}')

def make_setter(cap_name, name, size, bins):
    if name == 'padd':
        return
    print(f'static inline cap_t cap_{cap_name}_set_{name}(cap_t cap, uint64_t {name}) {{')
    make_assert(f"cap_is_type(cap, CAP_TYPE_{cap_name.upper()})")
    make_assert(f"({name} & 0x{'ff'*size}ull) == {name}")
    i, b = get_bin(bins, name)
    offset = get_offset(b, name)
    if i == 0:
        offset += 8
    mask = f"0x{'ff'*size + '00'*(offset//8)}ull"
    if offset:
        print(f"cap.word{i} = (cap.word{i} & ~{mask}) | {name} << {offset};")
    else:
        print(f"cap.word{i} = (cap.word{i} & ~{mask}) | {name};")
    print("return cap;")
    print("}")

def make_constructor(cap_name, fields, asserts, bins):
    parameters = [f"uint64_t {name}" for (name, size) in fields if name != 'padd']
    print(f"static inline cap_t cap_mk_{cap_name}({', '.join(parameters)}) {{")
    for (name, size) in fields:
        if name != 'padd':
            make_assert(f"({name} & 0x{'ff'*size}ull) == {name}")
    for a in asserts:
        make_assert(a)
    print("cap_t c;")
    print(f"c.word0 = (uint64_t)CAP_TYPE_{cap_name.upper()};")
    print(f"c.word1 = 0;")
    for (i,b) in enumerate(bins):
        for (f, s) in b:
            if f == 'padd':
                continue
            offset = get_offset(b, f)
            if i == 0:
                offset += 8
            if offset:
                print(f"c.word{i} |= {f} << {offset};")
            else:
                print(f"c.word{i} |= {f};")
    print("return c;")
    print("}")

def make_revokable(caps):
    print("static inline int cap_is_revokable(cap_t cap) {")
    p = [f"cap_is_type(cap, CAP_TYPE_{cap['name'].upper()})" for cap in caps if cap['revokable']]
    print(f"return {'&&'.join(p)};")
    print("}")

def make_cap_functions(data):
    if 'fields' not in data:
        return
    cap_name = data['name']
    fields = [tuple(f.split()) for f in data['fields']]
    fields = [(n, int(s)) for n, s in fields]
    bins = binpack([7,8], fields)
    asserts = data['asserts'] if 'asserts' in data else []
    make_constructor(cap_name, fields, asserts, bins)
    for (f, s) in fields:
        make_getter(cap_name, f, s, bins)
        make_setter(cap_name, f, s, bins)

def make_translator(case):
    pairs = []
    for rel in case['conditions']:
        pairs += re.findall(r'\w:[a-z]+', rel)
    pairs = list(set(pairs))
    dic = dict()
    for pair in pairs:
        v,f = pair.split(':')
        if v == 'p':
            dic[pair] = f"cap_{case['parent']}_get_{f}(p)"
        if v == 'c':
            dic[pair] = f"cap_{case['child']}_get_{f}(c)"
    rels = []
    for rel in case['conditions']:
        for key, val in dic.items():
            rel = rel.replace(key, val)
        rels.append("(" + rel + ")")
    case['conditions'] = rels

def make_pred_case(case):
    parent_type = f"CAP_TYPE_{case['parent'].upper()}"
    child_type = f"CAP_TYPE_{case['child'].upper()}"
    make_translator(case)
    print(f"if (cap_is_type(p,{parent_type}) && cap_is_type(c, {child_type}))")
    print(f"return {'&&'.join(case['conditions'])};")

def make_pred(p):
    name=p['name']
    print(f"static inline int cap_{name}(cap_t p, cap_t c) {{")
    for case in p['cases']:
        make_pred_case(case)
    print("return 0;")
    print("}")
    

# Open the file and load the file

with open(sys.argv[1]) as f:
    data = yaml.load(f, Loader=SafeLoader)

caps = data['caps']
enums = ", ".join([f"CAP_TYPE_{d['name'].upper()}"  for d in caps])

print(f"""\
#pragma once
#include "config.h"
#include "kassert.h"
#include "pmp.h"

#define NULL_CAP ((cap_t){{0,0}})

typedef enum cap_type cap_type_t;
typedef struct cap cap_t;

enum cap_type {{
{enums}, NUM_OF_CAP_TYPES
}};

struct cap {{
unsigned long long word0, word1;
}};

static inline cap_type_t cap_get_type(cap_t cap) {{
return 0xffull & cap.word0;
}}

static inline int cap_is_type(cap_t cap, cap_type_t t) {{
    return cap_get_type(cap) == t;
}}
""")

for i in caps:
    make_cap_functions(i)

make_revokable(caps)

for p in data['predicates']:
    make_pred(p)
