#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MEM_SIZE 65536

//mapeamento das flags
#define FLAG_Z 0x01
#define FLAG_N 0x02
#define FLAG_C 0X04
#define FLAG_O 0x08

//registradores
#define SP 14
#define PC 15

typedef struct {
    uint16_t R[16];   
    uint16_t IR;          
    uint16_t FLAGS;        
    uint16_t MEM[MEM_SIZE];
} CPU;
//intrucoes
enum {
    OP_JMP  = 0x0,
    OP_JCND = 0x1,

    OP_ADDI = 0x2,
    OP_SUBI = 0x3,
    OP_LDR  = 0x4,
    OP_SHR  = 0x5,
    OP_SHL  = 0x6,

    OP_STR  = 0x7,
    OP_MOV  = 0x8,

    OP_ADD  = 0x9,
    OP_SUB  = 0xA,
    OP_AND  = 0xB,
    OP_OR   = 0xC,

    OP_CMP  = 0xD,
    OP_PUSH = 0xE,
    OP_POP  = 0xF,
};
//condicao de salto
enum {
    COND_JEQ = 0b00, // Z = 1
    COND_JNE = 0b01, // Z = 0
    COND_JLT = 0b10, // Z = 0 e C = 1
    COND_JGE = 0b11  // Z = 0 e C = 0
};

void load_program(CPU *cpu, const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        perror("Erro ao abrir arquivo");
        exit(1);
    }

    char line[256];

    while (fgets(line, sizeof(line), f)) {

        char *p = line;

        while (isspace(*p)) p++;
        if (*p == '\0' || *p == '/' || *p == ';')
            continue;

        unsigned addr, value;

        if (sscanf(p, "%x %x", &addr, &value) == 2) {
            cpu->MEM[addr & 0xFFFF] = value & 0xFFFF;
        }
    }

    fclose(f);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Uso: %s programa.txt\n", argv[0]);
        return 1;
    }

    CPU cpu = {0};

    cpu.R[SP] = 0xFFFE;
    cpu.R[PC] = 0x0000;

    load_program(&cpu, argv[1]);

    while (step(&cpu));

    return 0;
}







