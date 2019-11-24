/*****************************************************
chip8.h: header file for chip8.c
Author: TheDucker1
*****************************************************/

#ifndef CHIP_8_H_
#define CHIP_8_H_

#include<stdio.h>
#include<stdlib.h>

typedef struct CHIP8 {
  unsigned char memory[4096]; //4K memory
  unsigned char V[16]; //16 data registers
  unsigned short  I; //address register
  unsigned short pc; //program counter
  unsigned short stack[16];
  unsigned short sp; //stack pointer
  unsigned char delay_timer;
  unsigned char sound_timer;
  unsigned char gfx[64 * 32];
  unsigned short opcode;
  unsigned char key[16];
  unsigned char drawFlag;
  void (*loadGame)(); //load game
  void (*emulateCycle)(); //do a cycle
  void (*functionTable[16])(struct CHIP8 *);
  void (*SudoFunctionTable[16])(struct CHIP8 *);
  void (*ALUinstructionTable[16])(struct CHIP8 *);
  void (*KeyFunctionTable[16])(struct CHIP8 *);
  void (*AssignFunctionTable[16])(struct CHIP8 *);
  void (*AssignFunctionTableSubGet[16])(struct CHIP8 *);
  void (*AssignFunctionTableSubSet[16])(struct CHIP8 *);
} CHIP8;

CHIP8* CHIP8_init();
void CHIP8_loadGame(CHIP8* self, const char * path);
void CHIP8_emulateCycle(CHIP8* self);

void CHIP8_incrPC(CHIP8* self, const char value);
void CHIP8_unknownFunction(CHIP8* self);
void CHIP8_cls(CHIP8* self); //00E0
void CHIP8_return(CHIP8* self); //00EE
void CHIP8_gotoAddr(CHIP8* self); //1NNN
void CHIP8_gsubAddr(CHIP8* self); //2NNN
void CHIP8_condEqual(CHIP8* self); //3XNN
void CHIP8_condNEqual(CHIP8* self); //4XNN
void CHIP8_condCmp(CHIP8* self); //5XY0
void CHIP8_setVx(CHIP8* self); //6XNN
void CHIP8_addVx(CHIP8* self); //7XNN
void CHIP8_setVxToVy(CHIP8* self); //8XY0
void CHIP8_setVxOrVy(CHIP8* self); //8XY1
void CHIP8_setVxAndVy(CHIP8* self); //8XY2
void CHIP8_setVxXorVy(CHIP8* self); //8XY3
void CHIP8_addVxVy(CHIP8* self); //8XY4
void CHIP8_subVxVy(CHIP8* self); //8XY5
void CHIP8_lsbVx(CHIP8* self); //8XY6
void CHIP8_subVyVx(CHIP8* self); //8XY7
void CHIP8_msbVx(CHIP8* self); //8XYE
void CHIP8_condNCmp(CHIP8* self); //9XY0
void CHIP8_setMem(CHIP8* self); //ANNN
void CHIP8_jumpAddr(CHIP8* self); //BNNN
void CHIP8_rand(CHIP8* self); //CXNN
void CHIP8_draw(CHIP8* self); //DXYN
void CHIP8_keyCmp(CHIP8* self); //EX9E
void CHIP8_keyNCmp(CHIP8* self); //EXA1
void CHIP8_getDelay(CHIP8* self); //FX07
void CHIP8_getKey(CHIP8* self); //FX0A
void CHIP8_setDelay(CHIP8* self); //FX15
void CHIP8_setSound(CHIP8* self); //FX18
void CHIP8_addMem(CHIP8* self); //FX1E
void CHIP8_setMemSpr(CHIP8* self); //FX29
void CHIP8_setBCD(CHIP8* self); //FX33
void CHIP8_regDump(CHIP8* self); //FX55
void CHIP8_regLoad(CHIP8* self); //FX65

void CHIP8_execSudoFunctionTable(CHIP8* self);
void CHIP8_execALUinstructionTable(CHIP8* self);
void CHIP8_execKeyFunctionTable(CHIP8* self);
void CHIP8_execAssignFunctionTable(CHIP8* self);
void CHIP8_execAssignFunctionTableSubGet(CHIP8* self);
void CHIP8_execAssignFunctionTableSubSet(CHIP8* self);
#endif
