#include <stdio.h>
#include <stdint.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/termios.h>
#include <sys/mman.h>

#define MEMORY_MAX (1 << 16)
uint16_t memory[MEMORY_MAX];

// Registers
enum
{
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
    R_COUNT
};
uint16_t reg[R_COUNT];


// Instructionos
enum
{
    OP_BR = 0,
    OP_ADD,
    OP_LD,
    OP_ST,
    OP_JSR,
    OP_AND,
    OP_LDR,
    OP_STR,
    OP_RTI,
    OP_NOT,
    OP_LDI,
    OP_STI,
    OP_JMP,
    OP_RES,
    OP_LEA,
    OP_TRAP
};



// Condition Flags
enum
{
    FL_POS = 1 << 0,
    FL_ZRO = 1 << 1,
    FL_NEG = 1 << 2,
};


uint16_t sign_extend(uint16_t x, int bit_count)
{
    if ((x >> (bit_count - 1)) & 1) 
    {
        x |= (0xFFFF << bit_count);
    }
    return x;
}

void update_flags(uint16_t r)
{
    if (reg[r] == 0)
    {
        reg[R_COND] = FL_ZRO;
    }
    else if (reg[r] >> 15)
    {
        reg[R_COND] = FL_NEG;
    }
    else
    {
        reg[R_COND] = FL_POS;
    }
}


uint16_t mem_read(uint16_t addr)
{
    return 0;
}


void mem_write(uint16_t addr, uint16_t obj)
{
}


// Instruction Functions
void add(uint16_t instr)
{
    uint16_t r0 = (instr >> 9) & 0x7;
    uint16_t r1 = (instr >> 6) & 0x7;
    uint16_t imm_flag = (instr >> 5) & 0x1;

    if (imm_flag)
    {
        uint16_t imm5 = sign_extend(instr & 0x1F, 5);
        reg[r0] = reg[r1] + imm5;
    }
    else
    {
        uint16_t r2 = instr & 0x7;
        reg[r0] = reg[r1] + reg[r2];
    }

    update_flags(r0);
}


void ldi(uint16_t instr)
{
    uint16_t r0 = (instr >> 9) & 0x7;
    uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
    reg[r0] = mem_read(mem_read(reg[R_PC] + pc_offset));
    update_flags(r0);
}


void and(uint16_t instr)
{
    uint16_t r0 = (instr >> 9) & 0x7;
    uint16_t r1 = (instr >> 6) & 0x7;
    uint16_t imm_flag = (instr >> 5) & 0x1;

    if (imm_flag)
    {
        uint16_t imm5 = sign_extend(instr & 0x1F, 5);
        reg[r0] = reg[r1] & imm5;
    }
    else
    {
        uint16_t r2 = instr & 0x7;
        reg[r0] = reg[r1] & reg[r2];
    }

    update_flags(r0);
}


void br(uint16_t instr)
{
    uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
    uint16_t cond_flag = (instr >> 9) & 0x7;
    if (cond_flag & reg[R_COND])
    {
        reg[R_PC] += pc_offset;
    }
}


void jmp(uint16_t instr)
{
    uint16_t r1 = (instr >> 6) & 0x7;
    reg[R_PC] = reg[r1];
}


void jsr(uint16_t instr)
{
    uint16_t long_flag = (instr >> 11) & 1;
    reg[R_R7] = reg[R_PC];
    if (long_flag)
    {
        uint16_t long_pc_offset = sign_extend(instr & 0x7FF, 11);
        reg[R_PC] += long_pc_offset;
    }
    else
    {
        uint16_t r1 = (instr >> 6) & 0x7;
        reg[R_PC] = reg[r1];
    }
}


void ld(uint16_t instr)
{
    uint16_t r0 = (instr >> 9) & 0x7;
    uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
    reg[r0] = mem_read(reg[R_PC] + pc_offset);
    update_flags(r0);
}


void ldr(uint16_t instr)
{
    uint16_t offset = sign_extend(instr & 0x3F, 6); 
    uint16_t r0 = (instr >> 9) & 0x7;
    uint16_t r1 = (instr >> 6) & 0x7;
    reg[r0] = mem_read(reg[r1] + offset);
    update_flags(r0);
}


void lea(uint16_t instr)
{
    uint16_t r0 = (instr >> 9) & 0x7;
    reg[r0] = reg[R_PC] + sign_extend(instr & 0x1FF, 9);
    update_flags(r0);
}


void not(uint16_t instr)
{
    uint16_t r0 = (instr >> 9) & 0x7;
    uint16_t r1 = (instr >> 6) & 0x7;
    reg[r0] = ~reg[r1];
    update_flags(r0);
}


void ret(uint16_t instr)
{
    reg[R_PC] = reg[R_R7];
}


void st(uint16_t instr)
{
    uint16_t r0 = (instr >> 9) & 0x7;
    uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
    mem_write(reg[R_PC] + pc_offset, reg[r0]);
}


void sti(uint16_t instr)
{
    uint16_t r0 = (instr >> 9) & 0x7;
    uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
    mem_write(mem_read(reg[R_PC] + pc_offset), reg[r0]);
}


void str(uint16_t instr)
{
    uint16_t r0 = (instr >> 9) & 0x7;
    uint16_t r1 = (instr >> 6) & 0x7;
    uint16_t offset = sign_extend(instr & 0x3F, 6);
    mem_write(r1 + offset, r0);
}


int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        // Show usage string
        printf("lc3 [image-file1] ...\n");
        exit(2);
    }

    // for (int i = 1; i < argc; ++i)
    // {
    //     if (!read_image(argv[i]))
    //     {
    //         printf("Failed to load image: %s\n", argv[i]);
    //         exit(1);
    //     }

    // }
    
    
    // set z condition flag
    reg[R_COND] = FL_ZRO;

    // set pc to starting position
    enum { PC_START = 0x3000 };
    reg[R_PC] = PC_START;
    
    int running = 1;
    while (running)
    {
        // FETCH
        // uint16_t instr = mem_read(reg[R_PC]++);
        uint16_t instr = 30;
        uint16_t op = instr >> 12;

        switch (op)
        {
            case OP_BR:
                br(instr);
                break;
            case OP_ADD:
                add(instr);
                break;
            case OP_AND:
                and(instr);
                break;
            case OP_LD:
                ld(instr);
                break;
            case OP_ST:
                break;
            case OP_JSR:
                jsr(instr);
                break;
            case OP_LDR:
                ldr(instr);
                break;
            case OP_STR:
            case OP_NOT:
            case OP_LDI:
                ldi(instr);
                break;
            case OP_STI:
            case OP_JMP:
            case OP_LEA:
                lea(instr);
                break;
            case OP_TRAP:
            case OP_RES:
            case OP_RTI:
            default:
                printf("Bad opcode provided\n");
                abort();
                break;
        }
    }
}


