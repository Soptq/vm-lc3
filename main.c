//
// Created by Soptq on 2020/7/16.
//

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* 10 registers: R0-R7, PC, COND */
enum {
    R_R0 = 0,
    R_R1,
    R_R2,
    R_R3,
    R_R4,
    R_R5,
    R_R6,
    R_R7,
    R_PC,
    R_COND,
    R_TOTAL     /* R_TOTAL tell us how many registers there are */
};

/* 16 opcodes */
enum {
    OP_BR = 0,      /* 0000     0       branch */
    OP_ADD,         /* 0001     1       add */
    OP_LD,          /* 0010     2       load */
    OP_ST,          /* 0011     3       store */
    OP_JSR,         /* 0100     4       jump register */
    OP_AND,         /* 0101     5       bitwise and */
    OP_LDR,         /* 0110     6       load register */
    OP_STR,         /* 0111     7       store register */
    OP_RTI,         /* 1000     8       unused */
    OP_NOT,         /* 1001     9       bitwise not */
    OP_LDI,         /* 1010     10      load indirect */
    OP_STI,         /* 1011     11      store indirect */
    OP_JMP,         /* 1100     12      jump */
    OP_RES,         /* 1101     13      reserved (unused) */
    OP_LEA,         /* 1110     14      load effective address */
    OP_TRAP,        /* 1111     15      execute trap */
};

/* 3 conditional flags */
enum {
    FL_POS = 1 << 0,    /* Positive,    001 */
    FL_ZRO = 1 << 1,    /* Zero,        010 */
    FL_NEG = 1 << 2,    /* Negative,    100 */
};

/* 6 traps, interrupt */
enum {
    TRAP_GETC = 0x20,   /* get character from keyboard, not echoed onto the terminal */
    TRAP_OUT = 0x21,    /* output a character */
    TRAP_PUTS = 0x22,   /* output a word string */
    TRAP_IN = 0x23,     /* get character from keyboard, echoed onto the terminal */
    TRAP_PUTSP = 0x24,  /* output a byte string */
    TRAP_HALT = 0x25    /* halt the program */
};

/* 2^16 memory locations */
u_int16_t memory[UINT16_MAX];

/* R_TOTAL registers */
uint16_t reg[R_TOTAL];

/* flag of whether the vm is running */
int running = 1;

/* extend sign number. filling in 0's for positive numbers and 1's for negative numbers. */
uint16_t sign_extend(uint16_t x, int bit_count){
    if ((x >> (bit_count - 1)) & 1) {   /* true if the first bit is 1 => negative */
        x |= (0xFFFF << bit_count);     /* filling 1's */
    }
    return x;
}

/* update the COND flags based on the value we write to the register r */
void update_flags(uint16_t r){
    if (reg[r] == 0) {
        reg[R_COND] = FL_ZRO;
    } else if (reg[r] >> 15) {  /* true if the first bit is 1 => negative */
        reg[R_COND] = FL_NEG;
    } else {
        reg[R_COND] = FL_POS;
    }
}

/* read instructions from PC */
uint16_t mem_read(uint16_t address) {
    uint16_t test;
    return test;
}

/* write instructions from PC */
void mem_write(uint16_t address, uint16_t value) {

}

/*
 * ADD R0 R1 R2; add the contents of R0 to R1 and store in R2
 * bits:   15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0
 * mode 1:  0  0  0  1    DR    SR1  0 0 0  SR2
 * mode 2:  0  0  0  1    DR    SR1  1   imm5
 * */
void instr_add(uint16_t instr) {
    uint16_t r0 = (instr >> 9) & 0x7;
    uint16_t r1 = (instr >> 6) & 0x7;
    uint16_t imm_flag = (instr >> 5) & 0x1;

    if (imm_flag) {
        uint16_t imm5 = sign_extend(instr & 0x1F, 5);
        reg[r0] = reg[r1] + imm5;
    } else {
        uint16_t r2 = instr & 0x7;
        reg[r0] = reg[r1] + reg[r2];
    }

    update_flags(r0);
}

/*
 * LDI R0 PCoffset; load the memory address of (PC + PCoffset) to the R2
 * bits:   15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0
 *          1  0  1  0    DR       PCoffset9
 * */
void instr_ldi(uint16_t instr){
    uint16_t r0 = (instr >> 9) * 0x7;
    uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
    reg[r0] = mem_read(mem_read(reg[R_PC + pc_offset]));
    update_flags(r0);
}

/*
 * AND R0, R1, R2;
 * bits:   15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0
 * mode 1:  0  0  0  1    DR    SR1  0 0 0  SR2
 * mode 2:  0  0  0  1    DR    SR1  1   imm5
 * */
void instr_and(uint16_t instr){
    uint16_t r0 = (instr >> 9) & 0x7;
    uint16_t r1 = (instr >> 6) & 0x7;
    uint16_t imm_flag = (instr >> 5) & 0x1;

    if (imm_flag) {
        uint16_t imm5 = sign_extend(instr & 0x1F, 5);
        reg[r0] = reg[r1] & imm5;
    } else {
        uint16_t r2 = instr & 0x7;
        reg[r0] = reg[r1] & reg[r2];
    }
    update_flags(r0);
}

/*
 * BR LABEL
 * bits:   15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0
 *         0  0  0  0  n  z       PCoffset9
 * */
void instr_branch(uint16_t instr){
    uint16_t br_flag = (instr >> 9) & 0x7;
    uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
    if (br_flag & reg[R_COND]) {
        reg[R_PC] += pc_offset;
    }
}

/*
 * JMP R0
 * bits:   15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0
 * mode 1:  1  1  0  0  0  0 0 BaseR 0 0 0 0 0 0
 * */
void instr_jump(uint16_t instr){
    uint16_t r0 = (instr >> 6) & 0x7;
    reg[R_PC] = reg[r0];
}

/*
 * JSR
 * bits:   15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0
 *          0  1  0  0  1        PCoffset11
 * */
void instr_jsr(uint16_t instr){
    reg[R_R7] = reg[R_PC];
    uint16_t jsr_flag = (instr >> 11) * 0x1;
    if (jsr_flag) {
        uint16_t pc_offset = sign_extend(instr & 0x7FF, 11);
        reg[R_PC] += pc_offset;
    } else {
        uint16_t r0 = (instr >> 6) & 0x7;
        reg[R_PC] = reg[r0];
    }
}

/*
 * LD r0, LABEL;
 * bits:   15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0
 *          0  0  1  0    DR      PCoffset9
 * */
void instr_ld(uint16_t instr){
    uint16_t r0 = (instr >> 9) & 0x7;
    uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
    reg[r0] = mem_read(reg[r0] + pc_offset);
    update_flags(r0);
}

/*
 * LDR r0, r1, LABEL;
 * bits:   15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0
 *          0  1  1  0    DR     R1   PCoffset6
 * */
void instr_ldr(uint16_t instr){
    uint16_t r0 = (instr >> 9) & 0x7;
    uint16_t r1 = (instr >> 6) & 0x7;
    uint16_t pc_offset = sign_extend(instr & 0x3F, 6);
    reg[r0] = mem_read(reg[r1] + pc_offset);
    update_flags(r0);
}

/*
 * LEA DR, LABEL;
 * bits:   15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0
 *          1  1  1  0    DR      PCoffset9
 * */
void instr_lea(uint16_t instr){
    uint16_t r0 = (instr >> 9) & 0x7;
    uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
    reg[r0] = reg[R_PC] + pc_offset;
    update_flags(r0);
}

/*
 * NOT DR, SR;
 * bits:   15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0
 *          1  0  0  1    DR     SR  1 1 1 1 1 1
 * */
void instr_not(uint16_t instr){
    uint16_t r0 = (instr >> 9) & 0x7;
    uint16_t r1 = (instr >> 6) & 0x7;
    reg[r0] = ~reg[r1];
    update_flags(r0);
}

/*
 * RES
 * bits:   15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0
 *          1  1  0  0  0  0 0 1 1 1 0 0 0 0 0 0
 * */
void instr_res(uint16_t instr){
    return;
}

/*
 * RTI
 * bits:   15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0
 *          1  0  0  0  0  0 0 0 0 0 0 0 0 0 0 0
 * */
void instr_rti(uint16_t instr){
    return;
}

/*
 * ST SR, LABEL
 * bits:   15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0
 *          0  0  1  1    SR        PCoffset9
 * */
void instr_st(uint16_t instr){
    uint16_t r0 = (instr >> 9) & 0x7;
    uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
    mem_write(reg[R_PC] + pc_offset, reg[r0]);
}

/*
 * STI SR, LABEL
 * bits:   15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0
 *          1  0  1  1    SR        PCoffset9
 * */
void instr_sti(uint16_t instr){
    uint16_t r0 = (instr >> 9) & 0x7;
    uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
    mem_write(mem_read(reg[R_PC] + pc_offset), reg[r0]);
}

/*
 * STR SR, BaseR, offset6
 * bits:   15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0
 *          0  1  1  1    SR   BaseR  PCoffset6
 * */
void instr_str(uint16_t instr){
    uint16_t r0 = (instr >> 9) & 0x7;
    uint16_t r1 = (instr >> 6) & 0x7;
    uint16_t pc_offset = sign_extend(instr & 0x3F, 6);
    mem_write(reg[r1] + pc_offset, reg[r0]);
}

void trap_getc(){
    reg[R_R0] = (uint16_t)getchar();
}

void trap_out(){
    putc((char)reg[R_R0], stdout);
    fflush(stdout);
}

void trap_puts(){
    uint16_t* c = memory + reg[R_R0];
    while (*c){
        putc((char)*c, stdout);
        ++c;
    }
    fflush(stdout);
}

void trap_in(){
    printf("Input a character: ");
    char c = getchar();
    putc(c, stdout);
    reg[R_R0] = (uint16_t)c;
}

void trap_putsp(){
    uint16_t* c = memory + reg[R_R0];
    while (*c) {
        char c1 = (*c) & (0xFF);
        putc(c1, stdout);
        char c2 = (*c) >> 8;
        putc(c2, stdout);
        ++c;
    }
    fflush(stdout);
}

void trap_halt(){
    puts("HALT");
    fflush(stdout);
    running = 0;
}

/*
 * TRAP trapvector8
 * bits:   15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0
 *          1  1  1  1  0  0 0 0   trapvect8
 * */
void instr_trap(uint16_t instr){
    reg[R_R7] = reg[R_PC];
    switch (instr & 0xFF) {
        case TRAP_GETC:
            trap_getc();
            break;
        case TRAP_OUT:
            trap_out();
            break;
        case TRAP_PUTS:
            trap_puts();
            break;
        case TRAP_IN:
            trap_in();
            break;
        case TRAP_PUTSP:
            trap_putsp();
            break;
        case TRAP_HALT:
            trap_halt();
            break;
    }
    reg[R_PC] = mem_read();
}


/*
 * Main Procedures:
 * 1. Load one instruction from memory at the address of the PC register.
 * 2. Increment the PC register.
 * 3. Look at the opcode to determine which type of instruction it should perform.
 * 4. Perform the instruction using the parameters in the instruction.
 * 5. Go back to step 1.
 * */
int main(int argc, const char* argv[]){
    /* handle command input */
    if (argc < 2) {
        /* output the usage of the app */
        printf("main [image-file1] ... \n");
        exit(2);
    }

    for (int j = 1; j < argc; ++j) {
        if (!read_image(argv[j])) {
            printf("failed to load image: %s\n", argv[j]);
            exit(1);
        }
    }

    /* default PC start position, 0x3000 */
    enum { PC_START = 0x3000 };
    reg[R_PC] = PC_START;

    running = 1;

    while(running) {
        /* fetch the command */
        uint16_t instr = mem_read(reg[R_PC]++);
        uint16_t op = instr >> 12;  /* first 4 bits of a command is the opcode */

        switch (op) {
            case OP_ADD:
                instr_add(instr);
                break;
            case OP_AND:
                instr_add(instr);
                break;
            case OP_BR:
                instr_branch(instr);
                break;
            case OP_JMP:
                instr_jump(instr);
                break;
            case OP_JSR:
                instr_jsr(instr);
                break;
            case OP_LD:
                instr_ld(instr);
                break;
            case OP_LDI:
                instr_ldi(instr);
                break;
            case OP_LDR:
                instr_ldr(instr);
                break;
            case OP_LEA:
                instr_lea(instr);
                break;
            case OP_NOT:
                instr_not(instr);
                break;
            case OP_RES:
                instr_res(instr);
                break;
            case OP_RTI:
                instr_rti(instr);
                break;
            case OP_ST:
                instr_st(instr);
                break;
            case OP_STI:
                instr_sti(instr);
                break;
            case OP_STR:
                instr_str(instr);
                break;
            case OP_TRAP:
                instr_trap(instr);
                break;
            default:
                break;
        }
    }

    return 0;
}
