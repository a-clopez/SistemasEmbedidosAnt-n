.syntax unified
.cpu cortex-m0plus
.thumb
.text

.global reverse_int_file_asm
.type reverse_int_file_asm, %function

reverse_int_file_asm:
    push {r4, lr}

    mov r1, r0

    ldr r2, =0x55555555
    movs r3, r1
    lsrs r3, #1
    ands r3, r2
    ands r1, r2
    lsls r1, #1
    orrs r1, r3

    ldr r2, =0x33333333
    movs r3, r1
    lsrs r3, #2
    ands r3, r2
    ands r1, r2
    lsls r1, #2
    orrs r1, r3

    ldr r2, =0x0F0F0F0F
    movs r3, r1
    lsrs r3, #4
    ands r3, r2
    ands r1, r2
    lsls r1, #4
    orrs r1, r3

    ldr r2, =0x00FF00FF
    movs r3, r1
    lsrs r3, #8
    ands r3, r2
    ands r1, r2
    lsls r1, #8
    orrs r1, r3

    movs r4, r1
    lsrs r4, #16
    lsls r1, #16
    orrs r1, r4

    mov r0, r1
    pop {r4, pc}

.size reverse_int_file_asm, . - reverse_int_file_asm
