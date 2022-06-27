#!/bin/python3
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
    print(f"\tkassert({s});")

def make_getter(cap_name, name, size, bins):
    if name == 'padd':
        return
    print(f"const static inline uint64_t cap_{cap_name}_get_{name}(const Cap cap) {{")
    make_assert(f"(cap.word0 >> 56) == CAP_{cap_name.upper()}")
    i, b = get_bin(bins, name)
    offset = get_offset(b, name)
    if offset:
        print(f"\treturn (cap.word{i} >> {offset}) & 0x{'ff'*size}ull;")
    else:
        print(f"\treturn cap.word{i} & 0x{'ff'*size}ull;")
    print('}\n')

def make_setter(cap_name, name, size, bins):
    if name == 'padd':
        return
    print(f'const static inline Cap cap_{cap_name}_set_{name}(const Cap cap, uint64_t {name}) {{')
    make_assert(f"(cap.word0 >> 56) == CAP_{cap_name.upper()}")
    make_assert(f"({name} & 0x{'ff'*size}ull) == {name}")
    i, b = get_bin(bins, name) 
    offset = get_offset(b, name)
    mask = f"0x{'ff'*size + '00'*(offset//8)}ull"
    print("\tCap c;")
    if i == 1:
        print(f"\tc.word0 = cap.word0;")
    if offset:
        print(f"\tc.word{i} = (cap.word{i} & ~{mask}) | {name} << {offset};")
    else:
        print(f"\tc.word{i} = (cap.word{i} & ~{mask}) | {name};")
    if i == 0:
        print(f"\tc.word1 = cap.word1;")
    print("\treturn c;\n}\n")

def make_constructor(cap_name, fields, asserts, bins):
    parameters = [f"uint64_t {name}" for (name, size) in fields if name != 'padd']
    print(f"const static inline Cap cap_mk_{cap_name}({', '.join(parameters)}) {{")
    for (name, size) in fields:
        if name != 'padd':
            make_assert(f"({name} & 0x{'ff'*size}ull) == {name}")
    for a in asserts:
        make_assert(a)
    print("\tCap c;")
    print(f"\tc.word0 = (uint64_t)CAP_{cap_name.upper()} << 56;")
    print(f"\tc.word1 = 0;")
    for (i,b) in enumerate(bins):
        for (f, s) in b:
            if f == 'padd':
                continue
            offset = get_offset(b, f)
            if offset:
                print(f"\tc.word{i} |= {f} << {offset};")
            else:
                print(f"\tc.word{i} |= {f};")
    print("\treturn c;")
    print("}\n")

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
    for rel in case['relations']:
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
    for rel in case['relations']:
        for key, val in dic.items():
            rel = rel.replace(key, val) 
        rels.append(rel)
    case['relations'] = rels
        
def make_pred_case(case):
    parent_type = f"CAP_{case['parent'].upper()}"
    child_type = f"CAP_{case['child'].upper()}"
    make_translator(case)
    print(f"\tif (parent_type == {parent_type} && child_type == {child_type}) {{")
    for rel in case['relations']:
        print(f"\t\tb &= {rel};")
    print("\t\treturn b;\n\t}")

def make_pred(name, cases):
    print(f"static inline bool cap_{name}(const Cap p, const Cap c) {{")
    print("\tbool b = true;")
    print("\tCapType parent_type = cap_get_type(p);")
    print("\tCapType child_type = cap_get_type(c);")
    for case in cases:
        make_pred_case(case)
    print("\treturn false;")
    print("}\n")

# Open the file and load the file

with open(sys.argv[1]) as f:
    data = yaml.load(f, Loader=SafeLoader)

caps = data['caps']
enums = ", ".join([f"CAP_{d['name'].upper()}"  for d in caps])
print(f"""\
#pragma once
#include "cap.h"
#include "kassert.h"
#include "pmp.h"

typedef enum cap_type CapType;

enum cap_type {{
    {enums}
}};

static inline CapType cap_get_type(const Cap cap) {{
    return (cap.word0 >> 56) & 0xff;
}}
""")

for i in caps:
    make_cap_functions(i)

make_pred('is_child', data['is_child'])
make_pred('can_derive', data['can_derive'])
