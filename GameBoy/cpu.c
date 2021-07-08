#include"cpu.h"

gbcpu_p CPU_init(void) {
    gbcpu_p cpu = (gbcpu_p)malloc(sizeof(gbcpu_t));
    
    cpu->flag_AF = 0;
    cpu->flag_BC = 0;
    cpu->flag_DE = 0;
    cpu->flag_HL = 0;
    
    cpu->sp = 0;
    cpu->pc = 0;
}

int util_flagZ(gbcpu_p cpu) {
    return ((cpu->flagAF & LO_BITMASK) >> 6) & BITMASK;
}
int util_flagN(gbcpu_p cpu) {
    return ((cpu->flagAF & LO_BITMASK) >> 5) & BITMASK;
}
int util_flagH(gbcpu_p cpu) {
    return ((cpu->flagAF & LO_BITMASK) >> 4) & BITMASK;
}
int util_flagC(gbcpu_p cpu) {
    return ((cpu->flagAF & LO_BITMASK) >> 3) & BITMASK;
}
uint8_t util_getA(gbcpu_p cpu) {
    return (cpu->flag_AF & HI_BITMASK) >> 8;
}
uint8_t util_getB(gbcpu_p cpu) {
    return (cpu->flag_BC & HI_BITMASK) >> 8;
}
uint8_t util_getC(gbcpu_p cpu) {
    return (cpu->flag_BC & LO_BITMASK);
}
uint8_t util_getD(gbcpu_p cpu) {
    return (cpu->flag_DE & HI_BITMASK) >> 8;
}
uint8_t util_getE(gbcpu_p cpu) {
    return (cpu->flag_DE & LO_BITMASK);
}
void util_setA(gbcpu_p cpu, uint8_t v) {
    cpu->flag_AF &= LO_BITMASK;
    cpu->flag_AF |= v << 8;
}
void util_setB(gbcpu_p cpu, uint8_t v) {
    cpu->flag_BC &= LO_BITMASK;
    cpu->flag_BC |= v << 8;
}
void util_setC(gbcpu_p cpu, uint8_t v) {
    cpu->flag_BC &= HI_BITMASK;
    cpu->flag_BC |= v;
}
void util_setD(gbcpu_p cpu, uint8_t v) {
    cpu->flag_DE &= LO_BITMASK;
    cpu->flag_DE |= v << 8;
}
void util_setE(gbcpu_p cpu, uint8_t v) {
    cpu->flag_DE &= HI_BITMASK;
    cpu->flag_DE |= v;
}
void util_setH(gbcpu_p cpu, uint8_t v) {
    cpu->flag_HL &= LO_BITMASK;
    cpu->flag_HL |= v << 8;
}
void util_setL(gbcpu_p cpu, uint8_t v) {
    cpu->flag_HL &= HI_BITMASK;
    cpu->flag_HL |= v;
}
uint16_t util_to_ui16(int input_size, BYTEp input) {
    if (input_size < 2) {
        return 0;
    }
    return (input[0] << 8) | input[1];
}

//unknown function
int func_unknown(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) {
    return 1;
}

//8-bit load
//0x02
int func_load_mem_bc_a(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    memory[cpu->flag_BC] = util_getA(cpu);
    return 1;
}
//0x12
int func_load_mem_de_a(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    memory[cpu->flag_DE] = util_getA(cpu);
    return 1;
}
//0x22
int func_load_mem_hl_inc_a(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    memory[cpu->flag_HL++] = util_getA(cpu);
    return 1;
}
//0x32
int func_load_mem_hl_dec_a(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    memory[cpu->flag_HL--] = util_getA(cpu);
    return 1;
}

//0x06
int func_load_b_d8(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 1) {
        return 0;
    }
    util_setB(cpu, input[0]);
    return 1;
}
//0x16
int func_load_d_d8(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 1) {
        return 0;
    }
    util_setD(cpu, input[0]);
    return 1;
}
//0x26
int func_load_h_d8(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 1) {
        return 0;
    }
    util_setH(cpu, input[0]);
    return 1;
}
//0x36
int func_load_mem_hl_d8(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 1) {
        return 0;
    }
    memory[cpu->flag_HL] = input[0];
    return 1;
}

//0x0A
int func_load_a_mem_bc(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setA(cpu, memory[cpu->flag_BC]);
    return 1;
}
//0x1A
int func_load_a_mem_de(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setA(cpu, memory[cpu->flag_DE]);
    return 1;
}
//0x2A
int func_load_a_mem_hl_inc(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setA(cpu, memory[cpu->flag_HL++]);
    return 1;
}
//0x3A
int func_load_a_mem_hl_dec(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setA(cpu, memory[cpu->flag_HL--]);
    return 1;
}

//0x0E
int func_load_c_d8(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 1) {
        return 0;
    }
    util_setC(cpu, input[0]);
    return 1;
}
//0x1E
int func_load_e_d8(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 1) {
        return 0;
    }
    util_setE(cpu, input[0]);
    return 1;
}
//0x2E
int func_load_l_d8(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 1) {
        return 0;
    }
    util_setL(cpu, input[0]);
    return 1;
}
//0x3E
int func_load_a_d8(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 1) {
        return 0;
    }
    util_setA(cpu, input[0]);
    return 1;
}

//0x40
int func_load_b_b(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setB(cpu, util_getB(cpu));
    return 1;
}
//0x41
int func_load_b_c(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setB(cpu, util_getC(cpu));
    return 1;
}
//0x42
int func_load_b_d(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setB(cpu, util_getD(cpu));
    return 1;
}
//0x43
int func_load_b_e(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setB(cpu, util_getE(cpu));
    return 1;
}
//0x44
int func_load_b_h(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setB(cpu, util_getH(cpu));
    return 1;
}
//0x45
int func_load_b_l(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setB(cpu, util_getL(cpu));
    return 1;
}
//0x46
int func_load_b_mem_hl(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setB(cpu, memory[cpu->flag_HL]);
    return 1;
}
//0x47
int func_load_b_a(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setB(cpu, util_getA(cpu));
    return 1;
}

//0x48
int func_load_c_b(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setC(cpu, util_getB(cpu));
    return 1;
}
//0x49
int func_load_c_c(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setC(cpu, util_getC(cpu));
    return 1;
}
//0x4A
int func_load_c_d(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setC(cpu, util_getD(cpu));
    return 1;
}
//0x4B
int func_load_c_e(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setC(cpu, util_getE(cpu));
    return 1;
}
//0x4C
int func_load_c_h(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setC(cpu, util_getH(cpu));
    return 1;
}
//0x4D
int func_load_c_l(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setC(cpu, util_getL(cpu));
    return 1;
}
//0x4E
int func_load_c_mem_hl(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setC(cpu, memory[cpu->flag_HL]);
    return 1;
}
//0x4F
int func_load_c_a(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setC(cpu, util_getA(cpu));
    return 1;
}

//0x50
int func_load_d_b(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setD(cpu, util_getB(cpu));
    return 1;
}
//0x51
int func_load_d_c(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setD(cpu, util_getC(cpu));
    return 1;
}
//0x52
int func_load_d_d(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setD(cpu, util_getD(cpu));
    return 1;
}
//0x53
int func_load_d_e(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setD(cpu, util_getE(cpu));
    return 1;
}
//0x54
int func_load_d_h(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setD(cpu, util_getH(cpu));
    return 1;
}
//0x55
int func_load_d_l(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setD(cpu, util_getL(cpu));
    return 1;
}
//0x56
int func_load_d_mem_hl(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setD(cpu, memory[cpu->flag_HL]);
    return 1;
}
//0x57
int func_load_d_a(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setD(cpu, util_getA(cpu));
    return 1;
}

//0x58
int func_load_e_b(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setE(cpu, util_getB(cpu));
    return 1;
}
//0x59
int func_load_e_c(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setE(cpu, util_getC(cpu));
    return 1;
}
//0x5A
int func_load_e_d(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setE(cpu, util_getD(cpu));
    return 1;
}
//0x5B
int func_load_e_e(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setE(cpu, util_getE(cpu));
    return 1;
}
//0x5C
int func_load_e_h(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setE(cpu, util_getH(cpu));
    return 1;
}
//0x5D
int func_load_e_l(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setE(cpu, util_getL(cpu));
    return 1;
}
//0x5E
int func_load_e_mem_hl(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setE(cpu, memory[cpu->flag_HL]);
    return 1;
}
//0x5F
int func_load_e_a(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setE(cpu, util_getA(cpu));
    return 1;
}

//0x60
int func_load_h_b(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setH(cpu, util_getB(cpu));
    return 1;
}
//0x61
int func_load_h_c(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setH(cpu, util_getC(cpu));
    return 1;
}
//0x62
int func_load_h_d(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setH(cpu, util_getD(cpu));
    return 1;
}
//0x63
int func_load_h_e(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setH(cpu, util_getE(cpu));
    return 1;
}
//0x64
int func_load_h_h(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setH(cpu, util_getH(cpu));
    return 1;
}
//0x65
int func_load_h_l(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setH(cpu, util_getL(cpu));
    return 1;
}
//0x66
int func_load_h_mem_hl(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setH(cpu, memory[cpu->flag_HL]);
    return 1;
}
//0x67
int func_load_h_a(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setH(cpu, util_getA(cpu));
    return 1;
}

//0x68
int func_load_l_b(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setL(cpu, util_getB(cpu));
    return 1;
}
//0x69
int func_load_l_c(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setL(cpu, util_getC(cpu));
    return 1;
}
//0x6A
int func_load_l_d(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setL(cpu, util_getD(cpu));
    return 1;
}
//0x6B
int func_load_l_e(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setL(cpu, util_getE(cpu));
    return 1;
}
//0x6C
int func_load_l_h(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setL(cpu, util_getH(cpu));
    return 1;
}
//0x6D
int func_load_l_l(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setL(cpu, util_getL(cpu));
    return 1;
}
//0x6E
int func_load_l_mem_hl(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setL(cpu, memory[cpu->flag_HL]);
    return 1;
}
//0x6F
int func_load_l_a(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setL(cpu, util_getA(cpu));
    return 1;
}

//0x70
int func_load_mem_hl_b(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    memory[flag_HL] = util_getB(cpu);
    return 1;
}
//0x71
int func_load_mem_hl_c(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    memory[flag_HL] = util_getC(cpu);
    return 1;
}
//0x72
int func_load_mem_hl_d(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    memory[flag_HL] = util_getD(cpu);
    return 1;
}
//0x73
int func_load_mem_hl_e(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    memory[flag_HL] = util_getE(cpu);
    return 1;
}
//0x74
int func_load_mem_hl_h(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    memory[flag_HL] = util_getH(cpu);
    return 1;
}
//0x75
int func_load_mem_hl_l(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    memory[flag_HL] = util_getL(cpu);
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
int func_load_mem_hl_a(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    memory[flag_HL] = util_getA(cpu);
    return 1;
}

//0x78
int func_load_a_b(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setA(cpu, util_getB(cpu));
    return 1;
}
//0x79
int func_load_a_c(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setA(cpu, util_getC(cpu));
    return 1;
}
//0x7A
int func_load_a_d(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setA(cpu, util_getD(cpu));
    return 1;
}
//0x7B
int func_load_a_e(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setA(cpu, util_getE(cpu));
    return 1;
}
//0x7C
int func_load_a_h(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setA(cpu, util_getH(cpu));
    return 1;
}
//0x7D
int func_load_a_l(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setA(cpu, util_getL(cpu));
    return 1;
}
//0x7E
int func_load_a_mem_hl(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setA(cpu, memory[cpu->flag_HL]);
    return 1;
}
//0x7F
int func_load_a_a(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setA(cpu, util_getA(cpu));
    return 1;
}

//0xE0
int func_load_mem_offset_a8_a(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 1) {
        return 0;
    }
    memory[0xFF + input[0]] = util_getA(cpu);
    return 1;
}
//0xF0
int func_load_a_mem_offset_a8(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 1) {
        return 0;
    }
    util_setA(cpu, memory[0xFF + input[0]]);
    return 1;
}

//0xE2
int func_load_offset_c_a(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    memory[0xFF + util_getC(cpu)] = util_getA(cpu);
    return 1;
}
//0xF2
int func_load_a_offset_c(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 0) {
        return 0;
    }
    util_setA(cpu, memory[0xFF + util_getC(cpu)]);
    return 1;
}

//0xEA
int func_load_mem_a16_a(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 2) {
        return 0;
    }
    memory[util_to_ui16(input_size, input)] = util_getA(cpu);
    return 1;
}
//0xFA
int func_load_a_mem_a16(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) { 
    if (input_size != 2) {
        return 0;
    }
    util_setA(cpu, memory[util_to_ui16(input_size, input)]);
    return 1;
}

//16-bit load
//0x01
int func_load_bc_d18(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) {
    if (input_size != 2) {
        return 0;
    }
    cpu->flag_BC = util_to_ui16(input_size, input);
    return 1;
}
//0x11
int func_load_de_d18(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) {
    if (input_size != 2) {
        return 0;
    }
    cpu->flag_DE = util_to_ui16(input_size, input);
    return 1;
}
//0x21
int func_load_hl_d18(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) {
    if (input_size != 2) {
        return 0;
    }
    cpu->flag_HL = util_to_ui16(input_size, input);
    return 1;
}
//0x31
int func_load_sp_d18(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) {
    if (input_size != 2) {
        return 0;
    }
    cpu->sp = util_to_ui16(input_size, input);
    return 1;
}

//0x80
int func_load_mem_a16_sp(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) {
    if (input_size != 2) {
        return 0;
    }
    memory[util_to_ui16(input_size, input)] = cpu->sp;
    return 1;
}

//0xC1
int func_pop_bc(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }
    util_setC(cpu, memory[cpu->sp++]);
    util_setB(cpu, memory[cpu->sp++]);
    return 1;
}
//0xD1
int func_pop_de(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }
    util_setC(cpu, memory[cpu->sp++]);
    util_setB(cpu, memory[cpu->sp++]);
    return 1;
}
//0xE1
int func_pop_hl(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }
    util_setC(cpu, memory[cpu->sp++]);
    util_setB(cpu, memory[cpu->sp++]);
    return 1;
}
//0xF1
int func_pop_af(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }
    util_setC(cpu, memory[cpu->sp++]);
    util_setB(cpu, memory[cpu->sp++]);
    return 1;
}

//0xC5
int func_push_bc(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }
    memory[cpu->sp--] = util_getB(cpu);
    memory[cpu->sp--] = util_getC(cpu);
    return 1;
}
//0xD5
int func_push_de(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }
    memory[cpu->sp--] = util_getD(cpu);
    memory[cpu->sp--] = util_getE(cpu);
    return 1;
}
//0xE5
int func_push_hl(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }
    memory[cpu->sp--] = util_getH(cpu);
    memory[cpu->sp--] = util_getL(cpu);
    return 1;
}
//0xF5
int func_push_af(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }
    memory[cpu->sp--] = util_getA(cpu);
    memory[cpu->sp--] = util_getF(cpu);
    return 1;
}

//0xF8
int func_load_hl_sp_add_r8(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) {
    if (input_size != 1) {
        return 0;
    }
    cpu->sp += (int16_t)input[0];
    cpu->hl = cpu->sp;
    return 1;
}
//0xF9
int func_load_sp_hl(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) {
    if (input_size != 0) {
        return 0;
    }
    cpu->sp = cpu->hl;
    return 1;
}

//jump / call
//0x20
int func_jump_nz_r8(gbcpu_p cpu, BYTEp memory, int input_size, BYTEp input) {
    if (input_size != 1) {
        return 0;
    }
    if (!(util_flagZ(cpu))) {
        //jump
    }
    return 1;
}
