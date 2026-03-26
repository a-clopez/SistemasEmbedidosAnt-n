# Practica 2: reverse_int en ASM

## Taboa comparativa de ciclos


| Implementacion | Ciclos (1 chamada) | Saida para 0x12345678 |
|---|---:|---|
| C (`reverse_int_c`) | 298 | 0x1e6a2c48 |
| Inline ASM (`reverse_int_inline_asm`) | 54 | 0x1e6a2c48 |
| ASM `.s` (`reverse_int_file_asm`) | 79 | 0x1e6a2c48 |
