#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MEM_SIZE 65536

//mapeamento das flags
#define FLAG_Z 0x01
#define FLAG_C 0X04

//registradores
#define SP 14
#define PC 15

typedef struct {
    uint16_t REG[16];   
    uint16_t IR;          
    uint16_t FLAGS;        
    uint16_t MEM[MEM_SIZE];
} IEMAS;

//intrucoes
enum {
    IEMAS_JMP  = 0x0,
    IEMAS_JCND = 0x1,

    IEMAS_ADDI = 0x2,
    IEMAS_SUBI = 0x3,
    IEMAS_LDR  = 0x4,
    IEMAS_SHR  = 0x5,
    IEMAS_SHL  = 0x6,

    IEMAS_STR  = 0x7,
    IEMAS_MOV  = 0x8,

    IEMAS_ADD  = 0x9,
    IEMAS_SUB  = 0xA,
    IEMAS_AND  = 0xB,
    IEMAS_OR   = 0xC,

    IEMAS_CMP  = 0xD,
    IEMAS_PUSH = 0xE,
    IEMAS_POP  = 0xF,

    IEMAS_JCO = 0001
};

/*
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
*/

void update_flags(IEMAS *cpu, uint32_t result) {
    cpu->FLAGS = 0;

    if ((result & 0xFFFF) == 0)
        cpu->FLAGS |= FLAG_Z;

    if (result > 0xFFFF)
        cpu.->FLAGS |= FLAG_C;

}

int main() {
    IEMAS cpu;

    cpu.REG[SP] = 0xFFFE;
    cpu.REG[PC] = 0x0000;

   while(true)  {
    uint16_t address;
    uint16_t line;
    scanf("%d", &address);
    scanf("%d", &line);

    cpu.MEM[address] = line;
    if(line == 0xFFFF)  {
        break;
    }
}

   bool loop = true;
   while(loop)  {
      // FETCH
      cpu.IR = cpu.MEM[cpu.REG[PC]];
      cpu.REG[PC] ++;

      // DECODING & EXECUTION
      uint16_t opc = cpu.IR & 0x000F; 

      switch(opc)  {
         case IEMAS_JMP:
            uint16_t imm = cpu.IR & 0xFFF0;
            uint16_t imm = ((int16_t) imm) >> 4;

            cpu.REG[PC] += imm;
            break;
         case IEMAS_JCO:
            uint16_t type = cpu.IR & 0x3000;
            uint16_t imm = cpu.IR & 0x3FF0;
            uint16_t imm =  imm << 2;
            uint16_t imm = ((int16_t) imm) >> 4;

            cpu.REG[PC] = imm;
            if(type == 0)  {
               if(cpu.FLAGS & FLAG_Z)  {
                  cpu.REG[PC] = imm;
               }
            } else if(type == 1) {
               if(!(cpu.FLAGS & FLAG_Z))  {
                  cpu.REG[PC] = imm;
               }
            } else if(type == 2) {
               if(cpu.FLAGS & FLAG_C)  {
                  cpu.REG[PC] = imm;
               }
            } else  {
               if(!(cpu.FLAGS & FLAG_C))  {
                  cpu.REG[PC] = imm;
               }
            }
            break;
         case IEMAS_LDR:
            uint16_t imm = cpu.IR & 0x00F0;
            uint16_t imm = ((int16_t) imm) >> 4;
            uint16_t rm = cpu.IR & 0x0F00;
            uint16_t rm = rm >> 8;
            uint16_t rd = cpu.IR & 0xF000;
            uint16_t rd = rd >> 12;

            cpu.REG[rd] = cpu.MEM[rm+imm];
            break;
         case IEMAS_STR:
            uint16_t rn = cpu.IR & 0x00F0;
            uint16_t rn = rn >> 4;
            uint16_t rm = cpu.IR & 0x0F00;
            uint16_t rm = rm >> 8;
            uint16_t imm = cpu.IR & 0xF000;
            uint16_t imm = ((int16_t) imm) >> 12;


            cpu.MEM[rm+imm] = rn;
            break;
         case IEMAS_MOV:
            uint16_t imm = cpu.IR & 0x00F0;
            uint16_t imm = ((int16_t) imm) >> 4;
            uint16_t rd = cpu.IR & 0xF000;
            uint16_t rd = rd >> 12;
            
            cpu.REG[rd] = imm;
            break;
         case IEMAS_ADD:
            uint16_t rn = cpu.IR & 0x00F0;
            uint16_t rn = ((int16_t) rn) >> 4;
            uint16_t rm = cpu.IR & 0x0F00;
            uint16_t rm = ((int16_t) rm) >> 8;
            uint16_t rd = cpu.IR & 0xF000;
            uint16_t rd = ((int16_t) rd) >> 12;
            
            int32_t result = cpu.REG[rm]+cpu.REG[rn];

            cpu.REG[rd] = result & 0xFFFF;
             
            update_flags(&cpu, result);
            break;
         case IEMAS_SUB:
            uint16_t rn = cpu.IR & 0x00F0;
            uint16_t rn = ((int16_t) rn) >> 4;
            uint16_t rm = cpu.IR & 0x0F00;
            uint16_t rm = ((int16_t) rm) >> 8;
            uint16_t rd = cpu.IR & 0xF000;
            uint16_t rd = ((int16_t) rd) >> 12;

            int32_t result = cpu.REG[rm]-cpu.REG[rn];

            cpu.REG[rd] = result & 0xFFFF;

            update_flags(&cpu, result);
            break;
         case IEMAS_AND:
            uint16_t rn = cpu.IR & 0x00F0;
            uint16_t rn = rn >> 4;
            uint16_t rm = cpu.IR & 0x0F00;
            uint16_t rm = rm >> 8;
            uint16_t rd = cpu.IR & 0xF000;
            uint16_t rd = rd >> 12;

            int32_t result = cpu.REG[rm]&cpu.REG[rn];

            cpu.REG[rd] = result & 0xFFFF;

            update_flags(&cpu, result);
            break;
         case IEMAS_OR:
            uint16_t rn = cpu.IR & 0x00F0;
            uint16_t rn = rn >> 4;
            uint16_t rm = cpu.IR & 0x0F00;
            uint16_t rm = rm >> 8;
            uint16_t rd = cpu.IR & 0xF000;
            uint16_t rd = rd >> 12;

            int32_t result = cpu.REG[rm]|cpu.REG[rn];

            cpu.REG[rd] = result & 0xFFFF;

            update_flags(&cpu, result);
            break;
         case IEMAS_ADDI:
            uint16_t imm = cpu.IR & 0x00F0;
            uint16_t imm = ((int16_t) imm) >> 4;
            uint16_t rm = cpu.IR & 0x0F00;
            uint16_t rm = ((int16_t) rm) >> 8;
            uint16_t rd = cpu.IR & 0xF000;
            uint16_t rd = ((int16_t)rd) >> 12;

            int32_t result = cpu.REG[rm]+imm;

            cpu.REG[rd] = result & 0xFFFF;

            update_flags(&cpu, result);
            break;
         case IEMAS_SUBI:
            uint16_t imm = cpu.IR & 0x00F0;
            uint16_t imm = ((int16_t) imm) >> 4;
            uint16_t rm = cpu.IR & 0x0F00;
            uint16_t rm = ((int16_t) rm) >> 8;
            uint16_t rd = cpu.IR & 0xF000;
            uint16_t rd = ((int16_t)rd) >> 12;

            int32_t result = cpu.REG[rm]-imm;

            cpu.REG[rd] = result & 0xFFFF;

            update_flags(&cpu, result);
            break;
         case IEMAS_SHR:
            uint16_t imm = cpu.IR & 0x00F0;
            uint16_t imm = ((int16_t) imm) >> 4;
            uint16_t rm = cpu.IR & 0x0F00;
            uint16_t rm = ((int16_t) rm) >> 8;
            uint16_t rd = cpu.IR & 0xF000;
            uint16_t rd = ((int16_t)rd) >> 12;

            int32_t result = cpu.REG[rm]<<imm;

            cpu.REG[rd] = result & 0xFFFF;

            update_flags(&cpu, result);
            break;
         case IEMAS_SHL:
            uint16_t imm = cpu.IR & 0x00F0;
            uint16_t imm = ((int16_t) imm) >> 4;
            uint16_t rm = cpu.IR & 0x0F00;
            uint16_t rm = rm >> 8;
            uint16_t rd = cpu.IR & 0xF000;
            uint16_t rd = rd >> 12;

            int32_t result = cpu.REG[rm]>>imm;

            cpu.REG[rd] = result & 0xFFFF;

            update_flags(&cpu, result);
            break;
         case IEMAS_CMP:
            uint16_t rn = cpu.IR & 0x00F0;
            uint16_t rn = rn >> 4;
            uint16_t rm = cpu.IR & 0x0F00;
            uint16_t rm = rm >> 8;

            cpu.FLAGS = 0;

            if(rm<rn)  {
               cpu.FLAGS |= FLAG_C;
            }
            if(rn == rm)  {
               cpu.FLAGS |= FLAG_Z;
            }
            break;
         case IEMAS_PUSH:
            uint16_t rn = cpu.IR & 0x00F0;
            uint16_t rn = rn >> 4;

            cpu.REG[SP] --;
            cpu.MEM[cpu.REG[SP]] = rn;
            break;
         case IEMAS_POP:
            if(cpu.IR == 0xFFFF)  { // HALT
               loop = false;
               break;
            }
            uint16_t rd = cpu.IR & 0xF000;
            uint16_t rd = rd >> 12;

            cpu.MEM[rd] = cpu.MEM[cpu.REG[SP]];
            cpu.REG[SP] ++;
            break;
            
            
     }
   }


    return 0;
}







