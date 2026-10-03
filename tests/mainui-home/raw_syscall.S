.syntax unified
.arm
.fpu vfpv3-d16
.text
.global raw_syscall
.type raw_syscall,%function
raw_syscall:
.fnstart
.cantunwind
push {r7,lr}
mov r7,r0
mov r0,r1
mov r1,r2
mov r2,r3
svc #0
pop {r7,pc}
.fnend
.section .note.GNU-stack,"",%progbits
.text
.global abi_activate
.type abi_activate,%function
abi_activate:
.fnstart
.save {r4-r11,r12,lr}
push {r4-r11,r12,lr}
vpush {d8-d15}
vmov.f64 d8,#1.0
vmov.f64 d9,d8
vmov.f64 d10,d8
vmov.f64 d11,d8
vmov.f64 d12,d8
vmov.f64 d13,d8
vmov.f64 d14,d8
vmov.f64 d15,d8
mov r4,#4
mov r5,#5
mov r6,#6
mov r7,#7
mov r8,#8
mov r9,#9
mov r10,#10
mov r11,#11
ldr r12,=0x27724
blx r12
cmp r4,#4
bne 1f
cmp r5,#5
bne 1f
cmp r6,#6
bne 1f
cmp r7,#7
bne 1f
cmp r8,#8
bne 1f
cmp r9,#9
bne 1f
cmp r10,#10
bne 1f
cmp r11,#11
bne 1f
vmov.f64 d0,#1.0
vcmp.f64 d8,d0
vmrs APSR_nzcv,FPSCR
bne 1f
vcmp.f64 d9,d0
vmrs APSR_nzcv,FPSCR
bne 1f
vcmp.f64 d10,d0
vmrs APSR_nzcv,FPSCR
bne 1f
vcmp.f64 d11,d0
vmrs APSR_nzcv,FPSCR
bne 1f
vcmp.f64 d12,d0
vmrs APSR_nzcv,FPSCR
bne 1f
vcmp.f64 d13,d0
vmrs APSR_nzcv,FPSCR
bne 1f
vcmp.f64 d14,d0
vmrs APSR_nzcv,FPSCR
bne 1f
vcmp.f64 d15,d0
vmrs APSR_nzcv,FPSCR
bne 1f
vpop {d8-d15}
pop {r4-r11,r12,pc}
1: mov r0,#99
vpop {d8-d15}
pop {r4-r11,r12,pc}
.fnend

.text
.global abi_startup
.type abi_startup,%function
abi_startup:
.fnstart
.cantunwind
push {r4-r12,lr}
mov r0,#12
mov r1,#13
mov r2,#14
mov r3,#15
mov r4,#16
mov r11,#17
ldr r12,=0x15a30
blx r12
cmp r0,#12
bne 9f
cmp r1,#13
bne 9f
cmp r2,#14
bne 9f
cmp r3,#15
bne 9f
cmp r4,#16
bne 9f
cmp r11,#0
bne 9f
ldr r0,=0x15a30
cmp r12,r0
bne 9f
mov r0,#1
pop {r4-r12,pc}
9: mov r0,#0
pop {r4-r12,pc}
.fnend
