#include<stdlib.h>
#include<stdint.h>
#include<limit.h>
#include"contypes.h"

#ifndef CPU_H_
#define CPU_H_

const uint16_t HI_BITMASK  = 0xff00;
const uint16_t LO_BITMASK  = 0x00ff;
const uint8_t  LHF_BITMASK = 0x0f;
const uint8_t  UHF_BITMASK = 0xf0;
const uint16_t    BITMASK  = 0x0001;
#define Z_SHIFT 6
#define N_SHIFT 5
#define H_SHIFT 4
#define C_SHIFT 3
const uint16_t  Z_BITMASK = 1 << Z_SHIFT;
const uint16_t  N_BITMASK = 1 << N_SHIFT;
const uint16_t  H_BITMASK = 1 << H_SHIFT;
const uint16_t  C_BITMASK = 1 << C_SHIFT;


typedef struct GBCPU {
    uint16_t reg_AF, reg_BC, reg_DE, reg_HL;
    //flag_AF: F: z-n-h-c register
    uint8_t flag_IME;
    uint16_t sp; //stack pointer
    uint16_t pc; //program counter
    
    BYTEp* memory;
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
