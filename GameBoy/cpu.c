/* REFERENCES:                                                      *
 * https://gbdev.io/gb-opcodes/optables/                            *
 * https://rgbds.gbdev.io/docs/v0.5.1/gbz80.7                       *
 * https://gbdev.io/pandocs/CPU_Instruction_Set.html                *
 *                                                                  */

#include"cpu.h"

gbcpu_p CPU_init(BYTEp* memory) {
    gbcpu_p cpu = (gbcpu_p)malloc(sizeof(gbcpu_t));
    
    cpu->reg_AF = 0;
    cpu->reg_BC = 0;
    cpu->reg_DE = 0;
    cpu->reg_HL = 0;
    cpu->flag_IME = 0;
    
    cpu->sp = 0;
    cpu->pc = 0;
    
    cpu->memory = memory;
    
    return cpu;
}

// util / helper

uint8_t util_msb_hi(uint16_t v) {
    return v >> 15;
}
uint8_t util_msb_lo(uint16_t v) {
    return (v & LO_BITMASK) >> 7;
}
uint8_t util_lsb_hi(uint16_t v) {
    return (v >> 8) & BITMASK;
}
uint8_t util_lsb_lo(uint16_t v) {
    return v & BITMASK;
}

uint8_t util_flagZ(gbcpu_p cpu) {
    return ((cpu->flagAF & Z_BITMASK) >> Z_SHIFT);
}
uint8_t util_flagN(gbcpu_p cpu) {
    return ((cpu->flagAF & N_BITMASK) >> N_SHIFT);
}
uint8_t util_flagH(gbcpu_p cpu) {
    return ((cpu->flagAF & H_BITMASK) >> H_SHIFT);
}
uint8_t util_flagC(gbcpu_p cpu) {
    return ((cpu->flagAF & C_BITMASK) >> C_SHIFT);
}
void util_setF(gbcpu_p cpu, uint8_t v) {
    cpu->reg_AF &= HI_BITMASK;
    cpu->reg_AF |= v;
}

uint8_t util_getA(gbcpu_p cpu) {
    return (cpu->reg_AF & HI_BITMASK) >> 8;
}
uint8_t util_getB(gbcpu_p cpu) {
    return (cpu->reg_BC & HI_BITMASK) >> 8;
}
uint8_t util_getC(gbcpu_p cpu) {
    return (cpu->reg_BC & LO_BITMASK);
}
uint8_t util_getD(gbcpu_p cpu) {
    return (cpu->reg_DE & HI_BITMASK) >> 8;
}
uint8_t util_getE(gbcpu_p cpu) {
    return (cpu->reg_DE & LO_BITMASK);
}
void util_setA(gbcpu_p cpu, uint8_t v) {
    cpu->reg_AF &= LO_BITMASK;
    cpu->reg_AF |= v << 8;
}
void util_setB(gbcpu_p cpu, uint8_t v) {
    cpu->reg_BC &= LO_BITMASK;
    cpu->reg_BC |= v << 8;
}
void util_setC(gbcpu_p cpu, uint8_t v) {
    cpu->reg_BC &= HI_BITMASK;
    cpu->reg_BC |= v;
}
void util_setD(gbcpu_p cpu, uint8_t v) {
    cpu->reg_DE &= LO_BITMASK;
    cpu->reg_DE |= v << 8;
}
void util_setE(gbcpu_p cpu, uint8_t v) {
    cpu->reg_DE &= HI_BITMASK;
    cpu->reg_DE |= v;
}
void util_setH(gbcpu_p cpu, uint8_t v) {
    cpu->reg_HL &= LO_BITMASK;
    cpu->reg_HL |= v << 8;
}
void util_setL(gbcpu_p cpu, uint8_t v) {
    cpu->reg_HL &= HI_BITMASK;
    cpu->reg_HL |= v;
}

uint16_t util_to_ui16(int input_size, BYTEp input) {
    if (input_size < 2) {
        return 0;
    }
    return (input[0] << 8) | input[1];
}

void func_call_helper(gbcpu_p cpu, uint16_t addr) {
    cpu->sp -= 2;
    (*cpu->memory)[cpu->sp] = cpu->pc;
    cpu->pc = addr;
}

void func_ret_helper(gbcpu_p cpu) {
    cpu->pc = (*cpu->memory)[cpu->sp];
    cpu->sp += 2;
}

void func_add_helper(gbcpu_p cpu, uint8_t v2) {
    uint8_t v1 = util_getA(cpu);
    uint8_t set_c = 0;
    uint8_t set_h = 0;
    if (0xFF - v2 < v1) {
        set_c = 1;
    }
    if (0x0F - (LHF_BITMASK & v2) < LHF_BITMASK & v1) {
        set_h = 1;
    }
    v1 = v1 + v2;
    util_setA(cpu, v1);
    util_setF(cpu, v1 == 0 << Z_SHIFT | set_h << H_SHIFT | set_c << C_SHIFT);
}

void func_adc_helper(gbcpu_p cpu, uint8_t v2) {
    uint8_t v1 = util_getA(cpu);
    uint8_t c = util_flagC(cpu);
    uint8_t set_c = 0;
    uint8_t set_h = 0;
    if (c) {
        if (0xFF - v2 <= v1) {
            set_c = 1;
        }
        if (0x0F - (LHF_BITMASK & v2) <= LHF_BITMASK & v1) {
            set_h = 1;
        }
    }
    else {
        if (0xFF - v2 < v1) {
            set_c = 1;
        }
        if (0x0F - (LHF_BITMASK & v2) < LHF_BITMASK & v1) {
            set_h = 1;
        }
    }
    v1 = v1 + v2 + c;
    util_setA(cpu, v1);
    util_setF(cpu, v1 == 0 << Z_SHIFT | set_h << H_SHIFT | set_c << C_SHIFT);
}

void func_sub_helper(gbcpu_p cpu, uint8_t v2) {
    uint8_t v1 = util_getA(cpu);
    uint8_t set_c = 0;
    uint8_t set_h = 0;
    if (v2 > v1) {
        set_c = 1;
    }
    if (v2 & LHF_BITMASK > v1 & LHF_BITMASK) {
        set_h = 1;
    }
    v1 = v1 - v2;
    util_setA(cpu, v1);
    util_setF(cpu, v1 == 0 << Z_SHIFT | 1 << N_SHIFT | set_h << H_SHIFT | set_c << C_SHIFT);
}

void func_sbc_helper(gbcpu_p cpu, uint8_t v2) {
    uint8_t v1 = util_getA(cpu);
    uint8_t c = util_flagC(cpu);
    uint8_t set_c = 0;
    uint8_t set_h = 0;
    if (v2 + c > v1) { //if v2 + c overflow??
        set_c = 1;
    }
    if ((v2 + c) & LHF_BITMASK > v1 & LHF_BITMASK) {
        set_h = 1;
    }
    v1 = v1 - v2 - c;
    util_setA(cpu, v1);
    util_setF(cpu, v1 == 0 << Z_SHIFT | 1 << N_SHIFT | set_h << H_SHIFT | set_c << C_SHIFT);
}

void func_and_helper(gbcpu_p cpu, uint8_t v2) {
    uint8_t v1 = util_getA(cpu);
    v1 &= v2;
    util_setA(cpu, v1);
    util_setF(cpu, v1 == 0 << Z_SHIFT | 1 << H_SHIFT);
}

void func_xor_helper(gbcpu_p cpu, uint8_t v2) {
    uint8_t v1 = util_getA(cpu);
    v1 ^= v2;
    util_setA(cpu, v1);
    util_setF(cpu, v1 == 0 << Z_SHIFT);
}

void func_or_helper(gbcpu_p cpu, uint8_t v2) {
    uint8_t v1 = util_getA(cpu);
    v1 |= v2;
    util_setA(cpu, v1);
    util_setF(cpu, v1 == 0 << Z_SHIFT);
}

void func_cp_helper(gbcpu_p cpu, uint8_t v2) {
    uint8_t v1 = util_getA(cpu);
    uint8_t set_c = 0;
    uint8_t set_h = 0;
    if (v2 > v1) {
        set_c = 1;
    }
    if (v2 & LHF_BITMASK > v1 & LHF_BITMASK) {
        set_h = 1;
    }
    v1 = v1 - v2;
    util_setF(cpu, v1 == 0 << Z_SHIFT | 1 << N_SHIFT | set_h << H_SHIFT | set_c << C_SHIFT);
}

void func_add_hl_helper(gbcpu_p cpu, uint16_t v2) {
    uint16_t v1 = cpu->reg_HL;
    uint8_t set_c = 0, set_h = 0;
    
    if (0xFFFF - v1 < v2) {
        set_c = 1;
    }
    if (0x0FFF - (v1 & 0x0FFF) < 0x0FFF & v2) {
        set_h = 1;
    }
    
    cpu->regHL = v1 + v2;
    util_setF(cpu, util_flagZ(cpu) << Z_SHIFT | set_h << H_SHIFT | set_c << C_SHIFT);
}

//unknown function
int func_unknown(gbcpu_p cpu, int input_size, BYTEp input) {
    return 1;
}

//8-bit load
//0x02
int func_load_mem_bc_a(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    (*cpu->memory)[cpu->reg_BC] = util_getA(cpu);
    return 1;
}
//0x12
int func_load_mem_de_a(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    (*cpu->memory)[cpu->reg_DE] = util_getA(cpu);
    return 1;
}
//0x22
int func_load_mem_hl_inc_a(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    (*cpu->memory)[cpu->reg_HL++] = util_getA(cpu);
    return 1;
}
//0x32
int func_load_mem_hl_dec_a(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    (*cpu->memory)[cpu->reg_HL--] = util_getA(cpu);
    return 1;
}

//0x06
int func_load_b_d8(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 1) {
        return 0;
    }
    util_setB(cpu, input[0]);
    return 1;
}
//0x16
int func_load_d_d8(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 1) {
        return 0;
    }
    util_setD(cpu, input[0]);
    return 1;
}
//0x26
int func_load_h_d8(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 1) {
        return 0;
    }
    util_setH(cpu, input[0]);
    return 1;
}
//0x36
int func_load_mem_hl_d8(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 1) {
        return 0;
    }
    (*cpu->memory)[cpu->reg_HL] = input[0];
    return 1;
}

//0x0A
int func_load_a_mem_bc(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setA(cpu, (*cpu->memory)[cpu->reg_BC]);
    return 1;
}
//0x1A
int func_load_a_mem_de(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setA(cpu, (*cpu->memory)[cpu->reg_DE]);
    return 1;
}
//0x2A
int func_load_a_mem_hl_inc(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setA(cpu, (*cpu->memory)[cpu->reg_HL++]);
    return 1;
}
//0x3A
int func_load_a_mem_hl_dec(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setA(cpu, (*cpu->memory)[cpu->reg_HL--]);
    return 1;
}

//0x0E
int func_load_c_d8(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 1) {
        return 0;
    }
    util_setC(cpu, input[0]);
    return 1;
}
//0x1E
int func_load_e_d8(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 1) {
        return 0;
    }
    util_setE(cpu, input[0]);
    return 1;
}
//0x2E
int func_load_l_d8(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 1) {
        return 0;
    }
    util_setL(cpu, input[0]);
    return 1;
}
//0x3E
int func_load_a_d8(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 1) {
        return 0;
    }
    util_setA(cpu, input[0]);
    return 1;
}

//0x40
int func_load_b_b(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setB(cpu, util_getB(cpu));
    return 1;
}
//0x41
int func_load_b_c(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setB(cpu, util_getC(cpu));
    return 1;
}
//0x42
int func_load_b_d(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setB(cpu, util_getD(cpu));
    return 1;
}
//0x43
int func_load_b_e(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setB(cpu, util_getE(cpu));
    return 1;
}
//0x44
int func_load_b_h(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setB(cpu, util_getH(cpu));
    return 1;
}
//0x45
int func_load_b_l(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setB(cpu, util_getL(cpu));
    return 1;
}
//0x46
int func_load_b_mem_hl(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setB(cpu, (*cpu->memory)[cpu->reg_HL]);
    return 1;
}
//0x47
int func_load_b_a(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setB(cpu, util_getA(cpu));
    return 1;
}

//0x48
int func_load_c_b(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setC(cpu, util_getB(cpu));
    return 1;
}
//0x49
int func_load_c_c(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setC(cpu, util_getC(cpu));
    return 1;
}
//0x4A
int func_load_c_d(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setC(cpu, util_getD(cpu));
    return 1;
}
//0x4B
int func_load_c_e(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setC(cpu, util_getE(cpu));
    return 1;
}
//0x4C
int func_load_c_h(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setC(cpu, util_getH(cpu));
    return 1;
}
//0x4D
int func_load_c_l(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setC(cpu, util_getL(cpu));
    return 1;
}
//0x4E
int func_load_c_mem_hl(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setC(cpu, (*cpu->memory)[cpu->reg_HL]);
    return 1;
}
//0x4F
int func_load_c_a(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setC(cpu, util_getA(cpu));
    return 1;
}

//0x50
int func_load_d_b(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setD(cpu, util_getB(cpu));
    return 1;
}
//0x51
int func_load_d_c(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setD(cpu, util_getC(cpu));
    return 1;
}
//0x52
int func_load_d_d(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setD(cpu, util_getD(cpu));
    return 1;
}
//0x53
int func_load_d_e(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setD(cpu, util_getE(cpu));
    return 1;
}
//0x54
int func_load_d_h(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setD(cpu, util_getH(cpu));
    return 1;
}
//0x55
int func_load_d_l(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setD(cpu, util_getL(cpu));
    return 1;
}
//0x56
int func_load_d_mem_hl(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setD(cpu, (*cpu->memory)[cpu->reg_HL]);
    return 1;
}
//0x57
int func_load_d_a(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setD(cpu, util_getA(cpu));
    return 1;
}

//0x58
int func_load_e_b(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setE(cpu, util_getB(cpu));
    return 1;
}
//0x59
int func_load_e_c(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setE(cpu, util_getC(cpu));
    return 1;
}
//0x5A
int func_load_e_d(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setE(cpu, util_getD(cpu));
    return 1;
}
//0x5B
int func_load_e_e(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setE(cpu, util_getE(cpu));
    return 1;
}
//0x5C
int func_load_e_h(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setE(cpu, util_getH(cpu));
    return 1;
}
//0x5D
int func_load_e_l(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setE(cpu, util_getL(cpu));
    return 1;
}
//0x5E
int func_load_e_mem_hl(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setE(cpu, (*cpu->memory)[cpu->reg_HL]);
    return 1;
}
//0x5F
int func_load_e_a(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setE(cpu, util_getA(cpu));
    return 1;
}

//0x60
int func_load_h_b(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setH(cpu, util_getB(cpu));
    return 1;
}
//0x61
int func_load_h_c(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setH(cpu, util_getC(cpu));
    return 1;
}
//0x62
int func_load_h_d(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setH(cpu, util_getD(cpu));
    return 1;
}
//0x63
int func_load_h_e(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setH(cpu, util_getE(cpu));
    return 1;
}
//0x64
int func_load_h_h(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setH(cpu, util_getH(cpu));
    return 1;
}
//0x65
int func_load_h_l(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setH(cpu, util_getL(cpu));
    return 1;
}
//0x66
int func_load_h_mem_hl(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setH(cpu, (*cpu->memory)[cpu->reg_HL]);
    return 1;
}
//0x67
int func_load_h_a(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setH(cpu, util_getA(cpu));
    return 1;
}

//0x68
int func_load_l_b(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setL(cpu, util_getB(cpu));
    return 1;
}
//0x69
int func_load_l_c(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setL(cpu, util_getC(cpu));
    return 1;
}
//0x6A
int func_load_l_d(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setL(cpu, util_getD(cpu));
    return 1;
}
//0x6B
int func_load_l_e(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setL(cpu, util_getE(cpu));
    return 1;
}
//0x6C
int func_load_l_h(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setL(cpu, util_getH(cpu));
    return 1;
}
//0x6D
int func_load_l_l(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setL(cpu, util_getL(cpu));
    return 1;
}
//0x6E
int func_load_l_mem_hl(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setL(cpu, (*cpu->memory)[cpu->reg_HL]);
    return 1;
}
//0x6F
int func_load_l_a(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setL(cpu, util_getA(cpu));
    return 1;
}

//0x70
int func_load_mem_hl_b(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    (*cpu->memory)[reg_HL] = util_getB(cpu);
    return 1;
}
//0x71
int func_load_mem_hl_c(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    (*cpu->memory)[reg_HL] = util_getC(cpu);
    return 1;
}
//0x72
int func_load_mem_hl_d(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    (*cpu->memory)[reg_HL] = util_getD(cpu);
    return 1;
}
//0x73
int func_load_mem_hl_e(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    (*cpu->memory)[reg_HL] = util_getE(cpu);
    return 1;
}
//0x74
int func_load_mem_hl_h(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    (*cpu->memory)[reg_HL] = util_getH(cpu);
    return 1;
}
//0x75
int func_load_mem_hl_l(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    (*cpu->memory)[reg_HL] = util_getL(cpu);
    return 1;
}
//0x76
/*          *
 *          *
 *   HALT   *
 *          *
 *          *
 *          */
//0x77
int func_load_mem_hl_a(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    (*cpu->memory)[reg_HL] = util_getA(cpu);
    return 1;
}

//0x78
int func_load_a_b(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setA(cpu, util_getB(cpu));
    return 1;
}
//0x79
int func_load_a_c(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setA(cpu, util_getC(cpu));
    return 1;
}
//0x7A
int func_load_a_d(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setA(cpu, util_getD(cpu));
    return 1;
}
//0x7B
int func_load_a_e(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setA(cpu, util_getE(cpu));
    return 1;
}
//0x7C
int func_load_a_h(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setA(cpu, util_getH(cpu));
    return 1;
}
//0x7D
int func_load_a_l(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setA(cpu, util_getL(cpu));
    return 1;
}
//0x7E
int func_load_a_mem_hl(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setA(cpu, (*cpu->memory)[cpu->reg_HL]);
    return 1;
}
//0x7F
int func_load_a_a(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setA(cpu, util_getA(cpu));
    return 1;
}

//0xE0
int func_load_mem_offset_a8_a(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 1) {
        return 0;
    }
    (*cpu->memory)[0xFF + input[0]] = util_getA(cpu);
    return 1;
}
//0xF0
int func_load_a_mem_offset_a8(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 1) {
        return 0;
    }
    util_setA(cpu, (*cpu->memory)[0xFF + input[0]]);
    return 1;
}

//0xE2
int func_load_offset_c_a(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    (*cpu->memory)[0xFF + util_getC(cpu)] = util_getA(cpu);
    return 1;
}
//0xF2
int func_load_a_offset_c(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setA(cpu, (*cpu->memory)[0xFF + util_getC(cpu)]);
    return 1;
}

//0xEA
int func_load_mem_a16_a(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 2) {
        return 0;
    }
    (*cpu->memory)[util_to_ui16(input_size, input)] = util_getA(cpu);
    return 1;
}
//0xFA
int func_load_a_mem_a16(gbcpu_p cpu, int input_size, BYTEp input) { 
    if (input_size != 2) {
        return 0;
    }
    util_setA(cpu, (*cpu->memory)[util_to_ui16(input_size, input)]);
    return 1;
}

//16-bit load
//0x01
int func_load_bc_d18(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 2) {
        return 0;
    }
    cpu->reg_BC = util_to_ui16(input_size, input);
    return 1;
}
//0x11
int func_load_de_d18(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 2) {
        return 0;
    }
    cpu->reg_DE = util_to_ui16(input_size, input);
    return 1;
}
//0x21
int func_load_hl_d18(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 2) {
        return 0;
    }
    cpu->reg_HL = util_to_ui16(input_size, input);
    return 1;
}
//0x31
int func_load_sp_d18(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 2) {
        return 0;
    }
    cpu->sp = util_to_ui16(input_size, input);
    return 1;
}

//0x80
int func_load_mem_a16_sp(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 2) {
        return 0;
    }
    (*cpu->memory)[util_to_ui16(input_size, input)] = cpu->sp;
    return 1;
}

//0xC1
int func_pop_bc(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }
    util_setC(cpu, (*cpu->memory)[cpu->sp++]);
    util_setB(cpu, (*cpu->memory)[cpu->sp++]);
    return 1;
}
//0xD1
int func_pop_de(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }
    util_setC(cpu, (*cpu->memory)[cpu->sp++]);
    util_setB(cpu, (*cpu->memory)[cpu->sp++]);
    return 1;
}
//0xE1
int func_pop_hl(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }
    util_setC(cpu, (*cpu->memory)[cpu->sp++]);
    util_setB(cpu, (*cpu->memory)[cpu->sp++]);
    return 1;
}
//0xF1
int func_pop_af(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }
    util_setC(cpu, (*cpu->memory)[cpu->sp++]);
    util_setB(cpu, (*cpu->memory)[cpu->sp++]);
    return 1;
}

//0xC5
int func_push_bc(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }
    (*cpu->memory)[cpu->sp--] = util_getB(cpu);
    (*cpu->memory)[cpu->sp--] = util_getC(cpu);
    return 1;
}
//0xD5
int func_push_de(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }
    (*cpu->memory)[cpu->sp--] = util_getD(cpu);
    (*cpu->memory)[cpu->sp--] = util_getE(cpu);
    return 1;
}
//0xE5
int func_push_hl(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }
    (*cpu->memory)[cpu->sp--] = util_getH(cpu);
    (*cpu->memory)[cpu->sp--] = util_getL(cpu);
    return 1;
}
//0xF5
int func_push_af(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }
    (*cpu->memory)[cpu->sp--] = util_getA(cpu);
    (*cpu->memory)[cpu->sp--] = util_getF(cpu);
    return 1;
}

//0xF8
int func_load_hl_sp_add_r8(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 1) {
        return 0;
    }
    cpu->sp += (int16_t)input[0];
    cpu->hl = cpu->sp;
    return 1;
}
//0xF9
int func_load_sp_hl(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }
    cpu->sp = cpu->hl;
    return 1;
}

//jump / call
//0x20
int func_jmp_nz_r8(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 1) {
        return 0;
    }
    if (!(util_flagZ(cpu))) {
        cpu->pc += (int8_t)input[0];
    }
    return 1;
}
//0x30
int func_jmp_nc_r8(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 1) {
        return 0;
    }
    if (!(util_flagC(cpu))) {
        cpu->pc += (int8_t)input[0];
    }
    return 1;
}
//0x18
int func_jmp_r8(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 1) {
        return 0;
    }
    cpu->pc += (int8_t)input[0];
    return 1;
}
//0x28
int func_jmp_z_r8(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 1) {
        return 0;
    }
    if (util_flagZ(cpu)) {
        cpu->pc += (int8_t)input[0];
    }
    return 1;
}
//0x38
int func_jmp_c_r8(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 1) {
        return 0;
    }
    if (util_flagZ(cpu)) {
        cpu->pc += (int8_t)input[0];
    }
    return 1;
}

//0xC0
int func_ret_nz(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }
    if (!util_flagZ(cpu)) {
        func_ret_helper(cpu);
    }
    return 1;
}
//0xD0
int func_ret_nc(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }
    if (!util_flagC(cpu)) {
        func_ret_helper(cpu);
    }
    return 1;
}

//0xC2
int func_jmp_nz_a16(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 2) {
        return 0;
    }
    if (!util_flagZ(cpu)) {
        cpu->pc = util_to_ui16(input_size, input);
    }
    return 1;
}
//0xD2
int func_jmp_nc_a16(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 2) {
        return 0;
    }
    if (!util_flagC(cpu)) {
        cpu->pc = util_to_ui16(input_size, input);
    }
    return 1;
}

//0xC3
int func_jmp_a16(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 1) {
        return 0;
    }
    cpu->pc = util_to_ui16(input_size, input);
    return 1;
}

//0xC4
int func_call_nz_a16(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 2) {
        return 0;
    }
    if (!util_flagZ(cpu)) {
        func_call_helper(cpu, util_to_ui16(input_size, input));
    }
    return 1;
}
//0xD4
int func_call_nc_a16(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 2) {
        return 0;
    }
    if (!util_flagC(cpu)) {
        func_call_helper(cpu, util_to_ui16(input_size, input));
    }
    return 1;
}

//0xC7
int func_rst_00h(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }
    func_call_helper(cpu, 0x00);
    return 1;
}
//0xD7
int func_rst_10h(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }
    func_call_helper(cpu, 0x10);
    return 1;
}
//0xE7
int func_rst_20h(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }
    func_call_helper(cpu, 0x20);
    return 1;
}
//0xF7
int func_rst_30h(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }
    func_call_helper(cpu, 0x30);
    return 1;
}

//0xC8
int func_ret_z(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }
    if (util_flagZ(cpu)) {
        func_ret_helper(cpu);
    }
    return 1;
}
//0xD8
int func_ret_c(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }
    if (util_flagC(cpu)) {
        func_ret_helper(cpu);
    }
    return 1;
}

//0xC9
int func_ret(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }
    func_ret_helper(cpu);
    return 1;
}

//0xD9
int func_reti(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }
    func_ret_helper(cpu);
    cpu->flag_IME = 1;
    return 1;
}

//0xE9
int func_jmp_hl(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }
    cpu->pc = cpu->flagHL;
    return 1;
}

//0xCA
int func_jmp_z_a16(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 2) {
        return 0;
    }
    if (util_flagZ(cpu)) {
        cpu->pc = util_to_ui16(input_size, input);
    }
    return 1;
}

//0xDA
int func_jmp_c_a16(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 2) {
        return 0;
    }
    if (util_flagC(cpu)) {
        cpu->pc = util_to_ui16(input_size, input);
    }
    return 1;
}

//0xCC
int func_call_nz_a16(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 2) {
        return 0;
    }
    if (util_flagZ(cpu)) {
        func_call_helper(cpu, util_to_ui16(input_size, input));
    }
    return 1;
}
//0xDC
int func_call_nc_a16(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 2) {
        return 0;
    }
    if (util_flagC(cpu)) {
        func_call_helper(cpu, util_to_ui16(input_size, input));
    }
    return 1;
}

//0xCD
int func_call_a16(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 2) {
        return 0;
    }
    func_call_helper(cpu, util_to_ui16(input_size, input));
    return 1;
}

//0xCF
int func_rst_08h(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }
    func_call_helper(cpu, 0x08);
    return 1;
}
//0xDF
int func_rst_18h(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }
    func_call_helper(cpu, 0x18);
    return 1;
}
//0xEF
int func_rst_28h(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }
    func_call_helper(cpu, 0x28);
    return 1;
}
//0xFF
int func_rst_38h(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }
    func_call_helper(cpu, 0x38);
    return 1;
}

//8-bit shift / rotate / bit-instruct (no GBC yet)
//0x07
int func_rlca(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }
    util_setF(cpu, util_msb_hi(cpu->reg_AF) << C_SHIFT);
    util_setA(cpu, util_getA(cpu) << 1);
    return 1;
}
//0x17
int func_rla(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }
    uint8_t c = util_flagC(cpu);
    util_setF(cpu, util_msb_hi(cpu->reg_AF)) << C_SHIFT);
    util_setA(cpu, (util_getA(cpu) << 1) | c);
    return 1;
}

//0x0F
int func_rrca(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }
    util_setF(cpu, util_lsb_hi(cpu->reg_AF) << C_SHIFT);
    util_setA(cpu, util_getA(cpu) >> 1);
    return 1;
}
//0x1F
int func_rra(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }
    uint8_t c = util_flagC(cpu);
    util_setF(cpu, util_lsb_hi(cpu->reg_AF) << C_SHIFT);
    util_setA(cpu, ((util_getA(cpu) >> 1) | (c << 7));
    return 1;
}

//8-bit arithmetic / logical instructions
//0x04
int func_inc_b(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }
    uint8_t v = util_getB(cpu);
    util_setF(cpu, ((v == 0xff) << Z_SHIFT) | ((v & LHF_BITMASK == LHF_BITMASK) << H_SHIFT) | (util_flagC(cpu) << C_SHIFT));
    util_setB(cpu, ++v);
    return 1;
}
//0x14
int func_inc_d(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }
    uint8_t v = util_getD(cpu);
    util_setF(cpu, ((v == 0xff) << Z_SHIFT) | ((v & LHF_BITMASK == LHF_BITMASK) << H_SHIFT) | (util_flagC(cpu) << C_SHIFT));
    util_setD(cpu, ++v);
    return 1;
}
//0x24
int func_inc_h(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }
    uint8_t v = util_getH(cpu);
    util_setF(cpu, ((v == 0xff) << Z_SHIFT) | ((v & LHF_BITMASK == LHF_BITMASK) << H_SHIFT) | (util_flagC(cpu) << C_SHIFT));
    util_setH(cpu, ++v);
    return 1;
}
//0x34
int func_inc_mem_hl(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }
    uint8_t v = (*cpu->memory)[cpu->regHL];
    util_setF(cpu, ((v == 0xff) << Z_SHIFT) | ((v & LHF_BITMASK == LHF_BITMASK) << H_SHIFT) | (util_flagC(cpu) << C_SHIFT));
    (*cpu->memory)[cpu->regHL]++;
    return 1;
}

//0x05
int func_dec_b(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }
    uint8_t v = util_getB(cpu);
    util_setF(cpu, ((v == 0x01) << Z_SHIFT) | N_BITMASK | ((v & LHF_BITMASK == 0x0) << H_SHIFT) | (util_flagC(cpu) << C_SHIFT));
    util_setB(cpu, --v);
    return 1;
}
//0x15
int func_dec_d(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }
    uint8_t v = util_getD(cpu);
    util_setF(cpu, ((v == 0x01) << Z_SHIFT) | N_BITMASK | ((v & LHF_BITMASK == 0x0) << H_SHIFT) | (util_flagC(cpu) << C_SHIFT));
    util_setD(cpu, --v);
    return 1;
}
//0x25
int func_dec_h(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }
    uint8_t v = util_getH(cpu);
    util_setF(cpu, ((v == 0x01) << Z_SHIFT) | N_BITMASK | ((v & LHF_BITMASK == 0x0) << H_SHIFT) | (util_flagC(cpu) << C_SHIFT));
    util_setH(cpu, --v);
    return 1;
}
//0x35
int func_dec_mem_hl(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }
    uint8_t v = (*cpu->memory)[cpu->regHL];
    util_setF(cpu, ((v == 0x01) << Z_SHIFT) | N_BITMASK | ((v & LHF_BITMASK == 0x0) << H_SHIFT) | (util_flagC(cpu) << C_SHIFT));
    (*cpu->memory)[cpu->regHL]--;
    return 1;
}

//0x0C
int func_inc_c(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }
    uint8_t v = util_getC(cpu);
    util_setF(cpu, ((v == 0xff) << Z_SHIFT) | ((v & LHF_BITMASK == LHF_BITMASK) << H_SHIFT) | (util_flagC(cpu) << C_SHIFT));
    util_setC(cpu, ++v);
    return 1;
}
//0x1C
int func_inc_e(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }
    uint8_t v = util_getE(cpu);
    util_setF(cpu, ((v == 0xff) << Z_SHIFT) | ((v & LHF_BITMASK == LHF_BITMASK) << H_SHIFT) | (util_flagC(cpu) << C_SHIFT));
    util_setE(cpu, ++v);
    return 1;
}
//0x2C
int func_inc_l(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }
    uint8_t v = util_getL(cpu);
    util_setF(cpu, ((v == 0xff) << Z_SHIFT) | ((v & LHF_BITMASK == LHF_BITMASK) << H_SHIFT) | (util_flagC(cpu) << C_SHIFT));
    util_setL(cpu, ++v);
    return 1;
}
//0x3C
int func_inc_a(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }
    uint8_t v = util_getA(cpu);
    util_setF(cpu, ((v == 0xff) << Z_SHIFT) | ((v & LHF_BITMASK == LHF_BITMASK) << H_SHIFT) | (util_flagC(cpu) << C_SHIFT));
    util_setA(cpu, ++v);
    return 1;
}

//0x0D
int func_dec_c(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }
    uint8_t v = util_getC(cpu);
    util_setF(cpu, ((v == 0x01) << Z_SHIFT) | N_BITMASK | ((v & LHF_BITMASK == 0x0) << H_SHIFT) | (util_flagC(cpu) << C_SHIFT));
    util_setC(cpu, --v);
    return 1;
}
//0x1D
int func_dec_e(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }
    uint8_t v = util_getE(cpu);
    util_setF(cpu, ((v == 0x01) << Z_SHIFT) | N_BITMASK | ((v & LHF_BITMASK == 0x0) << H_SHIFT) | (util_flagC(cpu) << C_SHIFT));
    util_setE(cpu, --v);
    return 1;
}
//0x2D
int func_dec_l(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }
    uint8_t v = util_getL(cpu);
    util_setF(cpu, ((v == 0x01) << Z_SHIFT) | N_BITMASK | ((v & LHF_BITMASK == 0x0) << H_SHIFT) | (util_flagC(cpu) << C_SHIFT));
    util_setL(cpu, --v);
    return 1;
}
//0x3D
int func_dec_a(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }
    uint8_t v = util_getA(cpu);
    util_setF(cpu, ((v == 0x01) << Z_SHIFT) | N_BITMASK | ((v & LHF_BITMASK == 0x0) << H_SHIFT) | (util_flagC(cpu) << C_SHIFT));
    util_setA(cpu, --v);
    return 1;
}

//0x27
//ref: https://stackoverflow.com/questions/45227884/z80-daa-implementation-and-blarggs-test-rom-issues
int func_daa(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }
    uint8_t v = util_getA(cpu);
    uint8_t c = util_flagC(cpu);
    uint8_t h = util_flagH(cpu);
    uint8_t n = util_flagN(cpu);
    
    uint8_t set_c = 0;
    
    if (!(n)) {
        if (h || (v & LHF_BITMASK) > 0x09) {
            if (0xFF - 0x06 < v) {
                set_c = 1;
            }
            v += 0x06;
        }
        if (c || (v & UHF_BITMASK) > 0x90) {
            if (0xFF - 0x60 < v) {
                set_c = 1;
            }
            v += 0x60;
        }
    }
    else {
        if (h) {
            if (v < 0x06) {
                set_c = 1; //do we?
            }
            v -= 0x06;
            if (!(c)) {
                v &= LO_BITMASK;
            }
        }
        if (c) {
            if (v < 0x60) {
                set_c = 1;
            }
            v -= 0x60;
        }
    }
    
    util_setF(cpu, (v == 0) << Z_SHIFT | (util_flagN(cpu) << N_SHIFT) | set_c << C_SHIFT);
    util_setA(cpu, v);
    
    return 1;
}

//0x37
int func_scf(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }
    util_setF(cpu, util_flagZ(cpu) << Z_SHIFT | 1 << C_SHIFT);
    return 1;
}

//0x2F
int func_cpl(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }
    util_setA(cpu, util_getA(cpu) ^ 0xFF);
    util_setF(cpu, util_flagZ(cpu) << Z_SHIFT | 1 << N_SHIFT | 1 << H_SHIFT | util_flagC(cpu) << C_SHIFT);
    return 1;
}

//0x3F
int func_scf(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }
    util_setF(cpu, util_flagZ(cpu) << Z_SHIFT | (util_flagC(cpu) ^ 1) << C_SHIFT);
    return 1;
}

//0x80
int func_add_b(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }
    func_add_helper(cpu, util_getA(cpu), util_getB(cpu));
    return 1;
}
//0x81
int func_add_c(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }
    func_add_helper(cpu, util_getA(cpu), util_getC(cpu));
    return 1;
}
//0x82
int func_add_d(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }
    func_add_helper(cpu, util_getA(cpu), util_getD(cpu));
    return 1;
}
//0x83
int func_add_e(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }
    func_add_helper(cpu, util_getA(cpu), util_getE(cpu));
    return 1;
}
//0x84
int func_add_h(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }
    func_add_helper(cpu, util_getA(cpu), util_getH(cpu));
    return 1;
}
//0x85
int func_add_l(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }
    func_add_helper(cpu, util_getA(cpu), util_getL(cpu));
    return 1;
}
//0x86
int func_add_mem_hl(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }    
    func_add_helper(cpu, util_getA(cpu), (*cpu->memory)[cpu->reg_HL]);
    return 1;
}
//0x87
int func_add_a(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }
    func_add_helper(cpu, util_getA(cpu), util_getA(cpu));
    return 1;
}

//0x88
int func_adc_b(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }  
    func_adc_helper(cpu, util_getB(cpu));
    return 1;
}
//0x89
int func_adc_c(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }  
    func_adc_helper(cpu, util_getC(cpu));
    return 1;
}
//0x8A
int func_adc_d(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }  
    func_adc_helper(cpu, util_getD(cpu));
    return 1;
}
//0x8B
int func_adc_e(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }  
    func_adc_helper(cpu, util_getE(cpu));
    return 1;
}
//0x8C
int func_adc_h(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }  
    func_adc_helper(cpu, util_getH(cpu));
    return 1;
}
//0x8D
int func_adc_l(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }  
    func_adc_helper(cpu, util_getL(cpu));
    return 1;
}
//0x8E
int func_adc_mem_hl(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }  
    func_adc_helper(cpu, (*cpu->memory)[cpu->reg_HL]);
    return 1;
}
//0x8F
int func_adc_a(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }  
    func_adc_helper(cpu, util_getA(cpu));
    return 1;
}

//0x90
int func_sub_b(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }  
    func_sub_helper(cpu, util_getB(cpu));
    return 1;
}
}
//0x91
int func_sub_c(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }  
    func_sub_helper(cpu, util_getC(cpu));
    return 1;
}
//0x92
int func_sub_d(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }  
    func_sub_helper(cpu, util_getD(cpu));
    return 1;
}
//0x93
int func_sub_e(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }  
    func_sub_helper(cpu, util_getE(cpu));
    return 1;
}
//0x94
int func_sub_h(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }  
    func_sub_helper(cpu, util_getH(cpu));
    return 1;
}
//0x95
int func_sub_l(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }  
    func_sub_helper(cpu, util_getL(cpu));
    return 1;
}
//0x96
int func_sub_mem_hl(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }  
    func_sub_helper(cpu, (*cpu->memory)[cpu->reg_HL]);
    return 1;
}
//0x97
int func_sub_a(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }  
    func_sub_helper(cpu, util_getA(cpu));
    return 1;
}

//0x98
int func_sbc_b(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }  
    func_sbc_helper(cpu, util_getB(cpu));
    return 1;
}
}
//0x99
int func_sbc_c(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }  
    func_sbc_helper(cpu, util_getC(cpu));
    return 1;
}
//0x9A
int func_sbc_d(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }  
    func_sbc_helper(cpu, util_getD(cpu));
    return 1;
}
//0x9B
int func_sbc_e(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }  
    func_sbc_helper(cpu, util_getE(cpu));
    return 1;
}
//0x9C
int func_sbc_h(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }  
    func_sbc_helper(cpu, util_getH(cpu));
    return 1;
}
//0x9D
int func_sbc_l(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }  
    func_sbc_helper(cpu, util_getL(cpu));
    return 1;
}
//0x9E
int func_sbc_mem_hl(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }  
    func_sbc_helper(cpu, (*cpu->memory)[cpu->reg_HL]);
    return 1;
}
//0x9F
int func_sbc_a(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }  
    func_sbc_helper(cpu, util_getA(cpu));
    return 1;
}

//0xA0
int func_and_b(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }  
    func_and_helper(cpu, util_getB(cpu));
    return 1;
}
//0xA1
int func_and_c(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }  
    func_and_helper(cpu, util_getC(cpu));
    return 1;
}
//0xA2
int func_and_d(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }  
    func_and_helper(cpu, util_getD(cpu));
    return 1;
}
//0xA3
int func_and_e(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }  
    func_and_helper(cpu, util_getE(cpu));
    return 1;
}
//0xA4
int func_and_h(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }  
    func_and_helper(cpu, util_getH(cpu));
    return 1;
}
//0xA5
int func_and_l(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }  
    func_and_helper(cpu, util_getL(cpu));
    return 1;
}
//0xA6
int func_and_mem_hl(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }  
    func_and_helper(cpu, (*cpu->memory)[cpu->reg_HL]);
    return 1;
}
//0xA7
int func_and_a(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }  
    func_and_helper(cpu, util_getA(cpu));
    return 1;
}

//0xA8
int func_xor_b(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }  
    func_xor_helper(cpu, util_getB(cpu));
    return 1;
}
//0xA9
int func_xor_c(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }  
    func_xor_helper(cpu, util_getC(cpu));
    return 1;
}
//0xAA
int func_xor_d(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }  
    func_xor_helper(cpu, util_getD(cpu));
    return 1;
}
//0xAB
int func_xor_e(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }  
    func_xor_helper(cpu, util_getE(cpu));
    return 1;
}
//0xAC
int func_xor_h(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }  
    func_xor_helper(cpu, util_getH(cpu));
    return 1;
}
//0xAD
int func_xor_l(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }  
    func_xor_helper(cpu, util_getL(cpu));
    return 1;
}
//0xAE
int func_xor_mem_hl(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }  
    func_xor_helper(cpu, (*cpu->memory)[cpu->reg_HL]);
    return 1;
}
//0xAF
int func_xor_a(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }  
    func_xor_helper(cpu, util_getA(cpu));
    return 1;
}

//0xB0
int func_or_b(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }  
    func_or_helper(cpu, util_getB(cpu));
    return 1;
}
//0xB1
int func_or_c(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }  
    func_or_helper(cpu, util_getC(cpu));
    return 1;
}
//0xB2
int func_or_d(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }  
    func_or_helper(cpu, util_getD(cpu));
    return 1;
}
//0xB3
int func_or_e(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }  
    func_or_helper(cpu, util_getE(cpu));
    return 1;
}
//0xB4
int func_or_h(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }  
    func_or_helper(cpu, util_getH(cpu));
    return 1;
}
//0xB5
int func_or_l(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }  
    func_or_helper(cpu, util_getL(cpu));
    return 1;
}
//0xB6
int func_or_mem_hl(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }  
    func_or_helper(cpu, (*cpu->memory)[cpu->reg_HL]);
    return 1;
}
//0xB7
int func_or_a(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }  
    func_or_helper(cpu, util_getA(cpu));
    return 1;
}

//0xB8
int func_cp_b(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }  
    func_cp_helper(cpu, util_getB(cpu));
    return 1;
}
//0xB9
int func_cp_c(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }  
    func_cp_helper(cpu, util_getC(cpu));
    return 1;
}
//0xBA
int func_cp_d(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }  
    func_cp_helper(cpu, util_getD(cpu));
    return 1;
}
//0xBB
int func_cp_e(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }  
    func_cp_helper(cpu, util_getE(cpu));
    return 1;
}
//0xBC
int func_cp_h(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }  
    func_cp_helper(cpu, util_getH(cpu));
    return 1;
}
//0xBD
int func_cp_l(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }  
    func_cp_helper(cpu, util_getL(cpu));
    return 1;
}
//0xBE
int func_cp_mem_hl(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }  
    func_cp_helper(cpu, (*cpu->memory)[cpu->reg_HL]);
    return 1;
}
//0xBF
int func_cp_a(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }  
    func_cp_helper(cpu, util_getA(cpu));
    return 1;
}

//0xC6
int func_add_d8(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 1) {
        return 0;
    }  
    func_add_helper(cpu, input[0]);
    return 1;
}
//0xD6
int func_sub_d8(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 1) {
        return 0;
    }  
    func_sub_helper(cpu, input[0]);
    return 1;
}
//0xE6
int func_and_d8(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 1) {
        return 0;
    }  
    func_and_helper(cpu, input[0]);
    return 1;
}
//0xF6
int func_or_d8(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 1) {
        return 0;
    }  
    func_or_helper(cpu, input[0]);
    return 1;
}

//0xCE
int func_adc_d8(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 1) {
        return 0;
    }  
    func_adc_helper(cpu, input[0]);
    return 1;
}
//0xDE
int func_sbc_d8(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 1) {
        return 0;
    }  
    func_sbc_helper(cpu, input[0]);
    return 1;
}
//0xEE
int func_xor_d8(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 1) {
        return 0;
    }  
    func_xor_helper(cpu, input[0]);
    return 1;
}
//0xFE
int func_cp_d8(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 1) {
        return 0;
    }  
    func_cp_helper(cpu, input[0]);
    return 1;
}

//16-bit arithmetic / logical instructions
//0x03
int func_inc_bc(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }
    cpu->reg_BC++;
    return 1;
}
//0x13
int func_inc_de(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }
    cpu->reg_DE++;
    return 1;
}
//0x23
int func_inc_hl(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }
    cpu->reg_HL++;
    return 1;
}
//0x33
int func_inc_sp(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }
    cpu->sp++;
    return 1;
}

//0x0B
int func_dec_bc(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }
    cpu->reg_BC--;
    return 1;
}
//0x1B
int func_dec_de(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }
    cpu->reg_DE--;
    return 1;
}
//0x2B
int func_dec_hl(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }
    cpu->reg_HL--;
    return 1;
}
//0x3B
int func_dec_sp(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }
    cpu->sp--;
    return 1;
}

//0x0A
int func_add_hl_bc(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }
    func_add_hl_helper(cpu, cpu->reg_BC);
    return 1;
}
//0x1A
int func_add_hl_de(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }
    func_add_hl_helper(cpu, cpu->reg_DE);
    return 1;
}
//0x2A
int func_add_hl_hl(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }
    func_add_hl_helper(cpu, cpu->reg_HL);
    return 1;
}
//0x3A
int func_add_hl_bc(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }
    func_add_hl_helper(cpu, cpu->sp);
    return 1;
}

//0xE8
int func_add_sp_r8(gbcpu_p cpu, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }
    uint16_t v1 = cpu->sp;
    int8_t v2 = (int8_t)input[0];
    uint8_t set_c = 0, set_h = 0;
    
    if (v2 > 0) {
        if (0xFF - LO_BITMASK & v1 < v2) {
            set_c = 1;
        }
        if (0x000F - (v1 & LHF_BITMASK) < LHF_BITMASK & v2) {
            set_h = 1;
        }
    }
    else if (v2 < 0) { //do we have to deal with negative input?
        if (v1 < -v2) {
            set_c = 1;
        }
        if (v1 & LHF_BITMASK < (-v2) & LHF_BITMASK) {
            set_h = 1;
        }
    }
    
    cpu->regHL = v1 + v2;
    util_setF(cpu, set_h << H_SHIFT | set_c << C_SHIFT);
    
    reutrn 1;
}
