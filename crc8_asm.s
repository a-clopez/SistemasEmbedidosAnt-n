.thumb
.syntax unified

    .global crc8_asm
    .type crc8_asm, %function

crc8_asm:
    push {r4, r5, r6, r7}

    movs r2, #0              
    cmp r1, #0
    beq .crc8_done

    movs r7, #0xFF           

.crc8_byte_loop:
    ldrb r3, [r0]            
    adds r0, r0, #1          
    subs r1, r1, #1         

    @ Paso 1
    mov r5, r2               
    eors r5, r3            
    lsrs r5, r5, #7       
    lsls r2, r2, #1         
    ands r2, r7              
    rsbs r5, r5, #0        
    movs r6, #0x07
    ands r5, r6             
    eors r2, r5              
    lsls r3, r3, #1          
    ands r3, r7              

    @ Paso 2
    mov r5, r2
    eors r5, r3
    lsrs r5, r5, #7
    lsls r2, r2, #1
    ands r2, r7
    rsbs r5, r5, #0
    movs r6, #0x07
    ands r5, r6
    eors r2, r5
    lsls r3, r3, #1
    ands r3, r7

    @ Paso 3
    mov r5, r2
    eors r5, r3
    lsrs r5, r5, #7
    lsls r2, r2, #1
    ands r2, r7
    rsbs r5, r5, #0
    movs r6, #0x07
    ands r5, r6
    eors r2, r5
    lsls r3, r3, #1
    ands r3, r7

    @ Paso 4
    mov r5, r2
    eors r5, r3
    lsrs r5, r5, #7
    lsls r2, r2, #1
    ands r2, r7
    rsbs r5, r5, #0
    movs r6, #0x07
    ands r5, r6
    eors r2, r5
    lsls r3, r3, #1
    ands r3, r7

    @ Paso 5
    mov r5, r2
    eors r5, r3
    lsrs r5, r5, #7
    lsls r2, r2, #1
    ands r2, r7
    rsbs r5, r5, #0
    movs r6, #0x07
    ands r5, r6
    eors r2, r5
    lsls r3, r3, #1
    ands r3, r7

    @ Paso 6
    mov r5, r2
    eors r5, r3
    lsrs r5, r5, #7
    lsls r2, r2, #1
    ands r2, r7
    rsbs r5, r5, #0
    movs r6, #0x07
    ands r5, r6
    eors r2, r5
    lsls r3, r3, #1
    ands r3, r7

    @ Paso 7
    mov r5, r2
    eors r5, r3
    lsrs r5, r5, #7
    lsls r2, r2, #1
    ands r2, r7
    rsbs r5, r5, #0
    movs r6, #0x07
    ands r5, r6
    eors r2, r5
    lsls r3, r3, #1
    ands r3, r7

    @ Paso 8
    mov r5, r2
    eors r5, r3
    lsrs r5, r5, #7
    lsls r2, r2, #1
    ands r2, r7
    rsbs r5, r5, #0
    movs r6, #0x07
    ands r5, r6
    eors r2, r5

    cmp r1, #0
    bne .crc8_byte_loop

.crc8_done:
    movs r0, r2              @ Copiar CRC a r0
    uxtb r0, r0              @ Mascarar a 8 bits (uint8_t)
    pop {r4, r5, r6, r7}
    bx lr
