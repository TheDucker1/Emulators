#include<stdlib.h>
#include<stdint.h>
#include<limit.h>
#include"contypes.h"

#ifndef CPU_H_
#define CPU_H_

#define HI_BITMASK 0xff00
#define LO_BITMASK 0x00ff
#define BITMASK 0x1

typedef struct GBCPU {
    uint16_t flag_AF, flag_BC, flag_DE, flag_HL;
    //flag_AF: F: z-n-h-c register
    uint16_t sp; //stack pointer
    uint16_t pc; //program counter
} gbcpu_t;

typedef gbcpu_t* gbcpu_p;


uint8_t util_getA(gbcpu_p cpu);
uint8_t util_getB(gbcpu_p cpu);
uint8_t util_getC(gbcpu_p cpu);
uint8_t util_getD(gbcpu_p cpu);
uint8_t util_getE(gbcpu_p cpu);
uint8_t util_getH(gbcpu_p cpu);
uint8_t util_getL(gbcpu_p cpu);


//8-bit load
int func_load_bc_8bit(gbcpu_p cpu, BYTEp memory);

#endif
