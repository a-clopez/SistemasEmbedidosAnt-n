#ifndef LCD_H
#define LCD_H

#include <stdint.h>

void LCD_Init(void);
void LCD_DisplayValues(uint8_t pending, uint8_t producers, uint8_t consumers);

#endif /* LCD_H */
