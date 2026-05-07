# TRABALLO TUTELADO 1

## Optimización da assembly de CRC8

### Que se optimizou

A versión optimizada en ensamblador mantén a mesma matemática, pero elimina dúas fontes principais de sobrecarga:

1. **O bucle interior de 8 bits está completamente despregado**
   - En vez de iterar 8 veces por cada byte, o código escribe explicitamente os 8 pasos.
   - Así elimínanse o salto e o mantemento do contador dentro da parte máis quente do algoritmo.

2. **A aplicación do polinomio é sen ramificación**
   - A versión en C usa un `if` para decidir se se debe facer `crc ^= 0x07`.
   - A versión en ensamblador converte esa condición nunha máscara.
   - Cando o bit de realimentación é 0, a máscara convértese en `0x00`.
   - Cando o bit de realimentación é 1, a máscara convértese en `0x07`.
   - Deste xeito o código sempre pode executar a XOR sen usar unha ramificación condicional.



### Resultados 

   |         Variante          | Ticks | Microsegundos|  Speedup vs O0 |
   |---------------------------|-------|--------------|----------------|
   | **O0** (no optimization)  | 38838 |    12946     |    baseline    |
   | **Ofast** (aggressive opt)|  4616 |     1538     |**8.41x faster**|
   | **ASM** (hand-optimized)  |  6152 |     2050     |**6.31x faster**|


