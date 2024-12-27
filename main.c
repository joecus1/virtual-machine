// TODO: Need to add this to compiler path or something
#include <cstdint>

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
}


uint16_t sign_extend(uint16_t x, int bit_count)
{
    if ((x >> (bit_count - 1)) & 1)
    {
        x |= (OxFFFF << bit_count);
    }
    return x;
}


void update_flags(uint16_t r)
{
    if (reg[r] == 0)
    {
        reg[R_COND] = FL_ZRO;
    }
    elif (reg[r] >> 15)
    {
        reg[R_COND] = FL_NEG;
    }
    else
    {
        reg[R_COND] = FL_POS;
    }
}


void add(uint16_t instr)
{
    uint16_t r0 = (instr >> 9) & 0x7;
    uint16_t r1 = (instr >> 6) & 0x7;
    uint16_t imm_flag = (instr >> 5) & 0x1;

    if (imm_flag)
    {
        uint16_t imm5 = sign_extend(instr & 0x1F, 5);
        reg[r0] = reg[1] + imm5;
    }
    else
    {
        uint16_t r2 = instr & 0x7;
        reg[r0] = reg[r1] + reg[r2];
    }

    update_flags(r0);
}


int main(int argc, conost char* argv[])
{
    if (argc < 2)
    {
        // Show usage string
        printf("lc3 [image-file1] ...\n");
        exit(2);
    }

    for (int i = 1; j < argc; ++j)
    {
        if (!read_image(argv[j]))
        {
            printf("Failed to load image: %s\n", argv[j]);
            exit(1);
        }

    }
    
    
    // set z condition flag
    reg[R_COND] = FL_ZRO;

    // set pc to starting position
    enum { PC_START = 0x3000 };
    reg[R_PC] = PC_START
    
    int running = 1;
    while (running)
    {
        // FETCH
        uint16_t instr = mem_read(reg[R_PC]++);
        uint16_t op = instr >> 12;

        switch (op)
        {
            case OP_ADD:
                add(instr);
                break;
            case OP_AND:
                break;
            case OP_LD:
                break;
            case OP_ST:
                break;
            case OP_JSR:
                break;
            case OP_AND:
            case OP_LDR:
            case OP_STR:
            case OP_NOT:
            case OP_LDI:
            case OP_STI:
            case OP_JMP:
            case OP_LEA:
            case OP_TRAP:
            case OP_RES:
            case OP_RTI:
            default:
                printf("Bad opcode provided\n");
                break;
        }
    }
}


