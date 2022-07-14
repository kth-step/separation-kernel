
.macro zero_state
        li      x1,0
        li      x2,0
        li      x3,0
        li      x4,0
        li      x5,0
        li      x6,0
        li      x7,0
        li      x8,0
        li      x9,0
        li      x10,0
        li      x11,0
        li      x12,0
        li      x13,0
        li      x14,0
        li      x15,0
        li      x16,0
        li      x17,0
        li      x18,0
        li      x19,0
        li      x20,0
        li      x21,0
        li      x22,0
        li      x23,0
        li      x24,0
        li      x25,0
        li      x26,0
        li      x27,0
        li      x28,0
        li      x29,0
        li      x30,0
        li      x31,0
.endm

.macro load_gp
        .option push
        .option norelax
        la      gp,__global_pointer$
        .option pop
.endm

.macro load_sp
        la      sp,core_stack
        csrr    t0,mhartid
        addi    t0,t0,1
        slli    t0,t0,LOG_STACK_SIZE
        add     sp,sp,t0
.endm

.macro save_context
        sd      ra,(1*8)(tp)
        sd      sp,(2*8)(tp)
        sd      gp,(3*8)(tp)
        /*sd      tp,(4*8)(tp)*/
        sd      t0,(5*8)(tp)
        sd      t1,(6*8)(tp)
        sd      t2,(7*8)(tp)
        sd      s0,(8*8)(tp)
        sd      s1,(9*8)(tp)
        sd      a0,(10*8)(tp)
        sd      a1,(11*8)(tp)
        sd      a2,(12*8)(tp)
        sd      a3,(13*8)(tp)
        sd      a4,(14*8)(tp)
        sd      a5,(15*8)(tp)
        sd      a6,(16*8)(tp)
        sd      a7,(17*8)(tp)
        sd      s2,(18*8)(tp)
        sd      s3,(19*8)(tp)
        sd      s4,(20*8)(tp)
        sd      s5,(21*8)(tp)
        sd      s6,(22*8)(tp)
        sd      s7,(23*8)(tp)
        sd      s8,(24*8)(tp)
        sd      s9,(25*8)(tp)
        sd      s10,(26*8)(tp)
        sd      s11,(27*8)(tp)
        sd      t3,(28*8)(tp)
        sd      t4,(29*8)(tp)
        sd      t5,(30*8)(tp)
        sd      t6,(31*8)(tp)

        csrr    t0,mepc
        sd      t0,(0*8)(tp)
.endm

.macro restore_context
        ld      t0,(0*8)(tp)
        csrw    mepc,t0

        ld      t6,(31*8)(tp)
        ld      t5,(30*8)(tp)
        ld      t4,(29*8)(tp)
        ld      t3,(28*8)(tp)
        ld      s11,(27*8)(tp)
        ld      s10,(26*8)(tp)
        ld      s9,(25*8)(tp)
        ld      s8,(24*8)(tp)
        ld      s7,(23*8)(tp)
        ld      s6,(22*8)(tp)
        ld      s5,(21*8)(tp)
        ld      s4,(20*8)(tp)
        ld      s3,(19*8)(tp)
        ld      s2,(18*8)(tp)
        ld      a7,(17*8)(tp)
        ld      a6,(16*8)(tp)
        ld      a5,(15*8)(tp)
        ld      a4,(14*8)(tp)
        ld      a3,(13*8)(tp)
        ld      a2,(12*8)(tp)
        ld      a1,(11*8)(tp)
        ld      a0,(10*8)(tp)
        ld      s1,(9*8)(tp)
        ld      s0,(8*8)(tp)
        ld      t2,(7*8)(tp)
        ld      t1,(6*8)(tp)
        ld      t0,(5*8)(tp)
        /*ld      tp,(4*8)(tp)*/
        ld      gp,(3*8)(tp)
        ld      sp,(2*8)(tp)
        ld      ra,(1*8)(tp)
.endm
