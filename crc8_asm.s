.thumb
.syntax unified

    .global crc8_asm
    .type crc8_asm, %function

/*
 * Implementación optimizada de CRC8 en assembly.
 *
 * Mantén a mesma recorrencia CRC ca base en C, pero optimízase mediante:
 * - a eliminación do bucle interior de 8 bits,
 * - a eliminación da ramificación condicional por bit,
 * - e o mantemento de todo o cálculo en rexistros.
 *
 * Polinomio: 0x07
 */
crc8_asm:
    push {r4, r5, r6, r7}

    movs r2, #0              @ Acumulador CRC
    cmp r1, #0
    beq .crc8_done

    movs r7, #0xFF           @ Máscara para 8 bits

.crc8_byte_loop:
    ldrb r3, [r0]            @ Ler o seguinte byte de entrada
    adds r0, r0, #1          @ Avanzar o punteiro
    subs r1, r1, #1          @ Reducir a lonxitude restante

    @ Paso 1
    mov r5, r2               @ r5 = crc
    eors r5, r3              @ r5 ^= byte
    lsrs r5, r5, #7          @ r5 = bit de realimentación (0 ou 1)
    lsls r2, r2, #1          @ crc <<= 1
    ands r2, r7              @ crc &= 0xFF
    rsbs r5, r5, #0          @ r5 = 0x00000000 ou 0xFFFFFFFF
    movs r6, #0x07
    ands r5, r6              @ r5 = 0x00 ou 0x07
    eors r2, r5              @ crc ^= (feedback ? 0x07 : 0x00)
    lsls r3, r3, #1          @ byte <<= 1
    ands r3, r7              @ byte &= 0xFF

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
