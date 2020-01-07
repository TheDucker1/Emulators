#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>

typedef struct GAMEBOY {
  uint16_t AF, BC, DE, HL, SP, PC; //Regs, Stack Pointer, Program Counter
  uint8_t Memory[0xFFFF + 1];
  enum Flag {
    Z = (1 << 6); //Zero
    N = (1 << 5); //Subtract
    H = (1 << 4); //Half-carry
    C = (1 << 3); //Carry
  };
} GAMEBOY;
