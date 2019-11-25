/*****************************************************
chip8.c: CHIP8 parser
Author: TheDucker1
References:
-Wikipedia (https://en.wikipedia.org/wiki/CHIP-8)
-How to write an emulator (CHIP-8 interpreter) (http://www.multigesture.net/articles/how-to-write-an-emulator-chip-8-interpreter/)
-Cowgod's Chip-8 Technical Reference v1.0 (http://devernay.free.fr/hacks/chip8/C8TECH10.HTM)
*****************************************************/

#include"chip8.h"

CHIP8* CHIP8_init() {
  CHIP8* chip8 = (CHIP8*)malloc(sizeof(CHIP8));
  chip8->pc = 0x200;
  chip8->opcode = 0;
  chip8->I = 0;
  chip8->sp = 0;

  unsigned char fontset[80] = {
  0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
  0x20, 0x60, 0x20, 0x20, 0x70, // 1
  0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
  0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
  0x90, 0x90, 0xF0, 0x10, 0x10, // 4
  0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
  0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
  0xF0, 0x10, 0x20, 0x40, 0x40, // 7
  0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
  0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
  0xF0, 0x90, 0xF0, 0x90, 0x90, // A
  0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
  0xF0, 0x80, 0x80, 0x80, 0xF0, // C
  0xE0, 0x90, 0x90, 0x90, 0xE0, // D
  0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
  0xF0, 0x80, 0xF0, 0x80, 0x80  // F
  };
  for (int i = 0; i < 16; ++i) {
    chip8->stack[i] = 0;
    chip8->V[i] = 0;
  } //clear stack, V
  for (int i = 0; i < 64 * 32; ++i) {
    chip8->gfx[i] = 0;
  } //clear gfx
  for (int i = 0; i < 4096; ++i) {
    chip8->memory[i] = 0;
  } //clear memory
  for (int i = 0; i < 80; ++i) {
    chip8->memory[i] = fontset[i];
  } //load fontset

  chip8->delay_timer = 0;
  chip8->sound_timer = 0;

  chip8->loadGame = CHIP8_loadGame;
  chip8->emulateCycle = CHIP8_emulateCycle;
  void (*functionTable[16])(CHIP8 *) = {CHIP8_execSudoFunctionTable, CHIP8_gotoAddr, CHIP8_gsubAddr, CHIP8_condEqual,
                                       CHIP8_condNEqual, CHIP8_condCmp, CHIP8_setVx, CHIP8_addVx,
                                       CHIP8_execALUinstructionTable, CHIP8_condNCmp, CHIP8_setMem, CHIP8_jumpAddr,
                                       CHIP8_rand, CHIP8_draw, CHIP8_execKeyFunctionTable, CHIP8_execAssignFunctionTable};

  void (*SudoFunctionTable[16])(CHIP8 *) = {CHIP8_cls, CHIP8_unknownFunction, CHIP8_unknownFunction, CHIP8_unknownFunction,
                                           CHIP8_unknownFunction, CHIP8_unknownFunction, CHIP8_unknownFunction, CHIP8_unknownFunction,
                                           CHIP8_unknownFunction, CHIP8_unknownFunction, CHIP8_unknownFunction, CHIP8_unknownFunction,
                                           CHIP8_unknownFunction, CHIP8_unknownFunction, CHIP8_return, CHIP8_unknownFunction};

  void (*ALUinstructionTable[16])(CHIP8 *) = {CHIP8_setVxToVy, CHIP8_setVxOrVy, CHIP8_setVxAndVy, CHIP8_setVxXorVy,
                                             CHIP8_addVxVy, CHIP8_subVxVy, CHIP8_lsbVx, CHIP8_subVyVx,
                                             CHIP8_unknownFunction, CHIP8_unknownFunction, CHIP8_unknownFunction, CHIP8_unknownFunction,
                                             CHIP8_unknownFunction, CHIP8_unknownFunction, CHIP8_msbVx, CHIP8_unknownFunction};

  void (*KeyFunctionTable[16])(CHIP8 *) = {CHIP8_unknownFunction, CHIP8_unknownFunction, CHIP8_unknownFunction, CHIP8_unknownFunction,
                                          CHIP8_unknownFunction, CHIP8_unknownFunction, CHIP8_unknownFunction, CHIP8_unknownFunction,
                                          CHIP8_unknownFunction, CHIP8_keyCmp, CHIP8_keyNCmp, CHIP8_unknownFunction,
                                          CHIP8_unknownFunction, CHIP8_unknownFunction, CHIP8_unknownFunction, CHIP8_unknownFunction};

  void (*AssignFunctionTable[16])(CHIP8 *) = {CHIP8_execAssignFunctionTableSubGet, CHIP8_execAssignFunctionTableSubSet, CHIP8_setMemSpr, CHIP8_setBCD,
                                             CHIP8_unknownFunction, CHIP8_regDump, CHIP8_regLoad, CHIP8_unknownFunction,
                                             CHIP8_unknownFunction, CHIP8_unknownFunction, CHIP8_unknownFunction, CHIP8_unknownFunction,
                                             CHIP8_unknownFunction, CHIP8_unknownFunction, CHIP8_unknownFunction, CHIP8_unknownFunction};

  void (*AssignFunctionTableSubGet[16])(CHIP8 *) = {CHIP8_unknownFunction, CHIP8_unknownFunction, CHIP8_unknownFunction, CHIP8_unknownFunction,
                                                   CHIP8_unknownFunction, CHIP8_unknownFunction, CHIP8_unknownFunction, CHIP8_getDelay,
                                                   CHIP8_unknownFunction, CHIP8_unknownFunction, CHIP8_getKey, CHIP8_unknownFunction,
                                                   CHIP8_unknownFunction, CHIP8_unknownFunction, CHIP8_unknownFunction, CHIP8_unknownFunction};

  void (*AssignFunctionTableSubSet[16])(CHIP8 *) = {CHIP8_unknownFunction, CHIP8_unknownFunction, CHIP8_unknownFunction, CHIP8_unknownFunction,
                                                   CHIP8_unknownFunction, CHIP8_setDelay, CHIP8_unknownFunction, CHIP8_unknownFunction,
                                                   CHIP8_setSound, CHIP8_unknownFunction, CHIP8_unknownFunction, CHIP8_unknownFunction,
                                                   CHIP8_unknownFunction, CHIP8_unknownFunction, CHIP8_addMem, CHIP8_unknownFunction};

  for (unsigned char i = 0; i < 16; ++i) {
    chip8->functionTable[i] = functionTable[i];
    chip8->SudoFunctionTable[i] = SudoFunctionTable[i];
    chip8->ALUinstructionTable[i] = ALUinstructionTable[i];
    chip8->KeyFunctionTable[i] = KeyFunctionTable[i];
    chip8->AssignFunctionTable[i] = AssignFunctionTable[i];
    chip8->AssignFunctionTableSubGet[i] = AssignFunctionTableSubGet[i];
    chip8->AssignFunctionTableSubSet[i] = AssignFunctionTableSubSet[i];
  }

  return chip8;
}

void CHIP8_loadGame(CHIP8* self, const char * path) {
  FILE *ptr_input_file;
  long lSize;
  char * buffer;
  size_t size;

  ptr_input_file = fopen(path, "rb");	
  if (ptr_input_file == NULL) {fputs("File error: Input file doesn't exist\n", stderr); exit (1);}
  fseek(ptr_input_file, 0, SEEK_END);
  lSize = ftell(ptr_input_file);
  rewind(ptr_input_file);

  buffer = (char*) malloc (sizeof(char)*lSize);
  if (buffer == NULL) {fputs("Memory error: Not enough memory", stderr); exit(2);}

  size = fread(buffer, 1, lSize, ptr_input_file);
  if (size != lSize) {fputs("Reading error", stderr); exit(3);}
  
  for (unsigned int i = 0; i < size; ++i) {
    self->memory[i + 0x200] = buffer[i];
  }

  fclose(ptr_input_file);
  free(buffer);

}

void CHIP8_emulateCycle(CHIP8* self) {
  self->drawFlag = 0;
  //Fetch opcode
  self->opcode = self->memory[self->pc] << 8 | self->memory[self->pc + 1];
  //Execute opcode
  self->functionTable[(self->opcode & 0xF000) >> 12](self);
  //Update timer
  if (self->delay_timer > 0) {--self->delay_timer;}
  if (self->sound_timer > 0) {if (self->sound_timer == 1) {}; --self->sound_timer;}
  CHIP8_incrPC(self, 2);
}

void CHIP8_unknownFunction(CHIP8* self) {

}

void CHIP8_cls(CHIP8* self) { //00E0
  self->drawFlag=1;
  for (unsigned int i = 0; i < 64 * 32; ++i) {
    self->gfx[i] = 0;
  } //clear gfx
}

void CHIP8_return(CHIP8* self) { //00EE
  self->pc = self->stack[--self->sp];
}

void CHIP8_gotoAddr(CHIP8* self) { //1NNN
  self->pc = self->opcode & 0x0FFF;
  CHIP8_incrPC(self, -2);
}

void CHIP8_gsubAddr(CHIP8* self) { //2NNN
  self->stack[self->sp] = self->pc;
  ++self->sp;
  self->pc = self->opcode & 0xFFF;
  CHIP8_incrPC(self, -2);
}

void CHIP8_condEqual(CHIP8* self) { //3XNN
  if ((self->V[(self->opcode & 0x0F00) >> 8]) == (self->opcode & 0x00FF)) {
    CHIP8_incrPC(self, 2);
  }
}

void CHIP8_condNEqual(CHIP8* self) { //4XNN
  if ((self->V[(self->opcode & 0x0F00) >> 8]) != (self->opcode & 0x00FF)) {
    CHIP8_incrPC(self, 2);
  }
}

void CHIP8_condCmp(CHIP8* self) { //5XY0
  if ((self->V[(self->opcode & 0x0F00) >> 8]) == (self->V[(self->opcode & 0x00F0) >> 4])) {
    CHIP8_incrPC(self, 2);
  }
}

void CHIP8_setVx(CHIP8* self) { //6XNN
  self->V[(self->opcode & 0x0F00) >> 8] = self->opcode & 0x00FF;
}

void CHIP8_addVx(CHIP8* self) { //7XNN
  self->V[(self->opcode & 0x0F00) >> 8] += self->opcode & 0x00FF;
}

void CHIP8_setVxToVy(CHIP8* self) { //8XY0
  self->V[(self->opcode & 0x0F00) >> 8] = self->V[(self->opcode & 0x00F0) >> 4];
}

void CHIP8_setVxOrVy(CHIP8* self) { //8XY1
  self->V[(self->opcode & 0x0F00) >> 8] |= self->V[(self->opcode & 0x00F0) >> 4];
}

void CHIP8_setVxAndVy(CHIP8* self) { //8XY2
  self->V[(self->opcode & 0x0F00) >> 8] &= self->V[(self->opcode & 0x00F0) >> 4];
}

void CHIP8_setVxXorVy(CHIP8* self) { //8XY3
  self->V[(self->opcode & 0x0F00) >> 8] ^= self->V[(self->opcode & 0x00F0) >> 4];
}

void CHIP8_addVxVy(CHIP8* self) { //8XY4
  self->V[0xF] = 0;
  if ((self->V[(self->opcode & 0x0F00) >> 8]) > (0xFF - self->V[(self->opcode & 0x00F0) >> 4])) {
    self->V[0xF] = 1;
  }
  self->V[(self->opcode & 0x0F00) >> 8] += self->V[(self->opcode & 0x00F0) >> 4];
}

void CHIP8_subVxVy(CHIP8* self) { //8XY5
  self->V[0xF] = 0;
  if ((self->V[(self->opcode & 0x0F00) >> 8]) > (self->V[(self->opcode & 0x00F0) >> 4])) {
    self->V[0xF] = 1;
  }
  self->V[(self->opcode & 0x0F00) >> 8] -= self->V[(self->opcode & 0x00F0) >> 4];
}

void CHIP8_lsbVx(CHIP8* self) { //8XY6
  self->V[0xF] = self->V[(self->opcode & 0x0F00) >> 8] & 0x01;
  self->V[(self->opcode & 0x0F00) >> 8] >>= 1;
}

void CHIP8_subVyVx(CHIP8* self) { //8XY7
  self->V[0xF] = 0;
  if ((self->V[(self->opcode & 0x0F00) >> 8]) < (self->V[(self->opcode & 0x00F0) >> 4])) {
    self->V[0xF] = 1;
  }
  self->V[(self->opcode & 0x0F00) >> 8] = self->V[(self->opcode & 0x00F0) >> 4] - self->V[(self->opcode & 0x0F00) >> 8];
}

void CHIP8_msbVx(CHIP8* self) { //8XYE
  self->V[0xF] = (self->V[(self->opcode & 0x0F00) >> 8] >> 7);
  self->V[(self->opcode & 0x0F00) >> 8] <<= 1;
}

void CHIP8_condNCmp(CHIP8* self) { //9XY0
  if ((self->V[(self->opcode & 0x0F00) >> 8]) != (self->V[(self->opcode & 0x00F0) >> 4])) {
    CHIP8_incrPC(self, 2);
  }
}

void CHIP8_setMem(CHIP8* self) { //ANNN
  self->I = self->opcode & 0x0FFF;
}

void CHIP8_jumpAddr(CHIP8* self) { //BNNN
  self->pc = self->V[0] + (self->opcode & 0x0FFF);
  CHIP8_incrPC(self, -2);
}

void CHIP8_rand(CHIP8* self) { //CXNN
  self->V[(self->opcode & 0x0F00) >> 8] = (rand() % 256) & (self->opcode & 0x00FF);
}

void CHIP8_draw(CHIP8* self) { //DXYN

  unsigned short pixel; //checking collision

  self->V[0xF] = 0;

  for (unsigned char y = 0; y < (self->opcode & 0x000F); ++y) {
    pixel = self->memory[self->I + y];
    for (unsigned char x = 0; x < 8; ++x) {
      if ((self->V[((self->opcode & 0x0F00) >> 8)] + x + ((self->V[((self->opcode & 0x00F0) >> 4)] + y) * 64)) < 64 * 32) {
        if ((pixel & (0x80 >> x)) != 0) { //might be collision here || ex: pixel = 10010000, x = 3 (binary) --> 0x80 >> x = 00010000 --> pixel & (0x80 >> x) = 1 --> collision
          if (self->gfx[self->V[((self->opcode & 0x0F00) >> 8)] + x + ((self->V[((self->opcode & 0x00F0) >> 4)] + y) * 64)] == 1) {
            self->V[0xF] = 1;
          }
          self->gfx[self->V[((self->opcode & 0x0F00) >> 8)] + x + ((self->V[((self->opcode & 0x00F0) >> 4)] + y) * 64)] ^= 1;
        }
      }
    }
  }
  self->drawFlag = 1;
}

void CHIP8_keyCmp(CHIP8* self) { //EX9E
  if (self->key[self->V[(self->opcode & 0x0F00) >> 8]] != 0) {
    CHIP8_incrPC(self, 2);
  }
}

void CHIP8_keyNCmp(CHIP8* self) { //EXA1
  if (self->key[self->V[(self->opcode & 0x0F00) >> 8]] == 0) {
    CHIP8_incrPC(self, 2);
  }
}

void CHIP8_getDelay(CHIP8* self) { //FX07
  self->V[(self->opcode & 0x0F00) >> 8] = self->delay_timer;
}

void CHIP8_getKey(CHIP8* self) { //FX0A
  for (unsigned char i = 0; i < 16; ++i) {
    if (self->key[i] == 1) {
      self->V[(self->opcode & 0x0F00) >> 8] = i;
      return;
    }
  }
  CHIP8_incrPC(self, -2);
}

void CHIP8_setDelay(CHIP8* self) { //FX15
  self->delay_timer = self->V[(self->opcode & 0x0F00) >> 8];
}

void CHIP8_setSound(CHIP8* self) { //FX18
  self->sound_timer = self->V[(self->opcode & 0x0F00) >> 8];
}

void CHIP8_addMem(CHIP8* self) { //FX1E
  if ((self->I) > (0xFFF - self->V[(self->opcode & 0x0F00) >> 8])) {
    self->V[0xF] = 1;
  }
  else {
    self->V[0xF] = 0;
  } 
  self->I += self->V[(self->opcode & 0x0F00) >> 8];
}

void CHIP8_setMemSpr(CHIP8* self) { //FX29
  self->I = self->V[(self->opcode & 0x0F00) >> 8] * 5;
}

void CHIP8_setBCD(CHIP8* self) { //FX33
  self->memory[self->I] = self->V[(self->opcode & 0x0F00) >> 8] / 100;
  self->memory[self->I + 1] = (self->V[(self->opcode & 0x0F00) >> 8] / 10) % 10;
  self->memory[self->I + 2] = self->V[(self->opcode & 0x0F00) >> 8]  % 10;
}

void CHIP8_regDump(CHIP8* self) { //FX55
  for (unsigned char i = 0; i <= ((self->opcode & 0x0F00) >> 8); ++i) {
    self->memory[self->I + i] = self->V[i];
  }
  self->I += ((self->opcode & 0x0F00) >> 8) + 1; //original chip8 only
}

void CHIP8_regLoad(CHIP8* self) { //FX65
  for (unsigned char i = 0; i <= ((self->opcode & 0x0F00) >> 8); ++i) {
    self->V[i] = self->memory[self->I + i];
  }
  self->I += ((self->opcode & 0x0F00) >> 8) + 1; //original chip8 only
}

void CHIP8_incrPC(CHIP8* self, const char value) {
  self->pc += value;
}

void CHIP8_execSudoFunctionTable(CHIP8* self) {
  return self->SudoFunctionTable[(self->opcode & 0x000F)](self);
}

void CHIP8_execALUinstructionTable(CHIP8* self) {
  return self->ALUinstructionTable[(self->opcode & 0x000F)](self);
}

void CHIP8_execKeyFunctionTable(CHIP8* self) {
  return self->KeyFunctionTable[(self->opcode & 0x00F0) >> 4](self);
}

void CHIP8_execAssignFunctionTable(CHIP8* self) {
  return self->AssignFunctionTable[(self->opcode & 0x00F0) >> 4](self);
}

void CHIP8_execAssignFunctionTableSubGet(CHIP8* self) {
  return self->AssignFunctionTableSubGet[(self->opcode & 0x000F)](self);
}

void CHIP8_execAssignFunctionTableSubSet(CHIP8* self) {
  return self->AssignFunctionTableSubSet[(self->opcode & 0x000F)](self);
}
