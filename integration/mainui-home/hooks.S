.file "hooks.S"
.syntax unified
.arm
.text
.align 2
.global bf_activation_hook
.type bf_activation_hook,%function
bf_activation_hook:
.fnstart
.cantunwind
    push {r0-r3,r12,lr}
    ldr r0,[r11,#-64]
    ldr r1,[r11,#-20]
    bl bf_home_activate
    cmp r0,#3
    pop {r0-r3,r12,lr}
    beq 1f
    mov r3,#0                     @ displaced instruction, exact stock replay
    b native_activation_resume
1:  mov r3,#3
    b native_activation_return   @ existing function epilogue/result propagation
.fnend
.size bf_activation_hook,.-bf_activation_hook

.global bf_writer_hook
.type bf_writer_hook,%function
bf_writer_hook:
.fnstart
.cantunwind
    ldr r12,=0x188d0             @ only the audited native launch writer call
    cmp lr,r12
    bne 3f
    ldr r12,=bf_transaction
    ldr r12,[r12]
    cmp r12,#0
    beq 3f
    push {r0-r3,r12,lr}
    bl bf_writer_is_owned         @ same thread as our Home activation
    cmp r0,#0
    pop {r0-r3,r12,lr}
    bne bf_stage_native_command   @ tail call; original r0 command is intact
3:  push {r11,lr}                 @ replay stock writer prologue when not ours
    b native_writer_resume
.fnend
.size bf_writer_hook,.-bf_writer_hook

.global bf_publication_hook
.type bf_publication_hook,%function
bf_publication_hook:
.fnstart
.cantunwind
    push {r0-r3,r12,lr}
    bl bf_publish_gate
    cmp r0,#0
    pop {r0-r3,r12,lr}
    beq 2f
    ldr r3,[r11,#-136]            @ exact displaced native config load
    b native_recent_resume
2:  mvn r4,#0                     @ native launch failure, not result 3
    b native_launch_cleanup      @ native strings destroyed through stock code
.fnend
.size bf_publication_hook,.-bf_publication_hook

// ELF entry: preserve incoming argument registers and stack, replay mov fp,#0.
// No libc constructors, heap allocation or VFP operations in startup diagnostics.
.global bf_startup_hook
.type bf_startup_hook,%function
bf_startup_hook:
.fnstart
.cantunwind
    push {r0-r12,lr}
    bl bf_home_startup
    pop {r0-r12,lr}
    mov r11,#0
    b native_startup_resume
.fnend
.size bf_startup_hook,.-bf_startup_hook

.global bf_syscall3
.type bf_syscall3,%function
bf_syscall3:
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
.size bf_syscall3,.-bf_syscall3
.section .note.GNU-stack,"",%progbits
