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

//breakpoints
#define MAX_BP 16
uint16_t breakpoints[MAX_BP];
int num_bp = 0;

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

void update_flags(IEMAS *cpu, uint32_t result) {
    cpu->FLAGS = 0;

    if ((result & 0xFFFF) == 0)
        cpu->FLAGS |= FLAG_Z;

    if (result > 0xFFFF)
        cpu->FLAGS |= FLAG_C;

}

bool is_breakpoint(uint16_t address) {
    for (int i = 0; i < num_bp; i++) {
        if (breakpoints[i]+1 == address) {
            return true;
        }
    }
    return false;
}

int main() {
    IEMAS cpu;

    cpu.REG[SP] = 0xFFFE;
    cpu.REG[PC] = 0x0000;

   scanf("%d", &num_bp);
   for (int i = 0; i < num_bp; i++) {
      scanf("%hx", &breakpoints[i]);
   }

   bool inputcheck = true;

   while(inputcheck)  {
      uint16_t address;
      uint16_t line;

      scanf("%hx", &address);
      scanf("%hx", &line);

      inputcheck = !(address == 0x0000 && line == 0x0000);
      cpu.MEM[address] = line;
   }

   bool loop = true;
   while(loop)  {
      // FETCH
      cpu.IR = cpu.MEM[cpu.REG[PC]];

      cpu.REG[PC] ++;

      // DECODING & EXECUTION
      uint16_t opc = cpu.IR & 0x000F; 
      uint16_t imm = 0;
      uint16_t rn = 0;
      uint16_t rm = 0;
      uint16_t rd = 0;
      uint32_t result = 0;

      

      switch(opc)  {
         case IEMAS_JMP: {
            imm = cpu.IR & 0xFFF0;
            imm = ((int16_t) imm) >> 4;

            cpu.REG[PC] += imm;
            break;
         }
         case IEMAS_JCO: {
            uint16_t type = cpu.IR & 0x3000;
            imm = cpu.IR & 0x3FF0;
            imm =  imm << 2;
            imm = ((int16_t) imm) >> 4;
         
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
         }
         case IEMAS_LDR:{
            imm = cpu.IR & 0x00F0;
            imm = ((int16_t) imm) >> 4;
            rm = cpu.IR & 0x0F00;
            rm = rm >> 8;
            rd = cpu.IR & 0xF000;
            rd = rd >> 12;

            cpu.REG[rd] = cpu.MEM[rm+imm];
            break;
         }
         case IEMAS_STR:{
            rn = cpu.IR & 0x00F0;
            rn = rn >> 4;
            rm = cpu.IR & 0x0F00;
            rm = rm >> 8;
            imm = cpu.IR & 0xF000;
            imm = ((int16_t) imm) >> 12;


            cpu.MEM[rm+imm] = cpu.REG[rn];
            break;
         }
         case IEMAS_MOV:{
            imm = cpu.IR & 0x00F0;
            imm = ((int16_t) imm) >> 4;
            rd = cpu.IR & 0xF000;
            rd = rd >> 12;
            
            cpu.REG[rd] = imm;
            break;
         }
         case IEMAS_ADD: {
            rn = cpu.IR & 0x00F0;
            rn = ((int16_t) rn) >> 4;
            rm = cpu.IR & 0x0F00;
            rm = ((int16_t) rm) >> 8;
            rd = cpu.IR & 0xF000;
            rd = ((int16_t) rd) >> 12;
            
            result = cpu.REG[rm]+cpu.REG[rn];

            cpu.REG[rd] = result & 0xFFFF;
             
            update_flags(&cpu, result);
            break;
         }
         case IEMAS_SUB: {
            rn = cpu.IR & 0x00F0;
            rn = ((int16_t) rn) >> 4;
            rm = cpu.IR & 0x0F00;
            rm = ((int16_t) rm) >> 8;
            rd = cpu.IR & 0xF000;
            rd = ((int16_t) rd) >> 12;

            result = cpu.REG[rm]-cpu.REG[rn];

            cpu.REG[rd] = result & 0xFFFF;

            update_flags(&cpu, result);
            break;
         }
         case IEMAS_AND: {
            rn = cpu.IR & 0x00F0;
            rn = ((int16_t) rn) >> 4;
            rm = cpu.IR & 0x0F00;
            rm = ((int16_t) rm) >> 8;
            rd = cpu.IR & 0xF000;
            rd = ((int16_t) rd) >> 12;

            result = cpu.REG[rm]&cpu.REG[rn];

            cpu.REG[rd] = result & 0xFFFF;

            update_flags(&cpu, result);
            break;
         }
         case IEMAS_OR: {
            rn = cpu.IR & 0x00F0;
            rn = ((int16_t) rn) >> 4;
            rm = cpu.IR & 0x0F00;
            rm = ((int16_t) rm) >> 8;
            rd = cpu.IR & 0xF000;
            rd = ((int16_t) rd) >> 12;

            result = cpu.REG[rm]|cpu.REG[rn];

            cpu.REG[rd] = result & 0xFFFF;

            update_flags(&cpu, result);
            break;
         }
         case IEMAS_ADDI: {
            imm = cpu.IR & 0x00F0;
            imm = ((int16_t) imm) >> 4;
            rm = cpu.IR & 0x0F00;
            rm = ((int16_t) rm) >> 8;
            rd = cpu.IR & 0xF000;
            rd = ((int16_t)rd) >> 12;

            result = cpu.REG[rm]+imm;

            cpu.REG[rd] = result & 0xFFFF;

            update_flags(&cpu, result);
            break;
         }
         case IEMAS_SUBI: {
            imm = cpu.IR & 0x00F0;
            imm = ((int16_t) imm) >> 4;
            rm = cpu.IR & 0x0F00;
            rm = ((int16_t) rm) >> 8;
            rd = cpu.IR & 0xF000;
            rd = ((int16_t)rd) >> 12;

            result = cpu.REG[rm]-imm;

            cpu.REG[rd] = result & 0xFFFF;

            update_flags(&cpu, result);
            break;
         }
         case IEMAS_SHR: {
            imm = cpu.IR & 0x00F0;
            imm = ((int16_t) imm) >> 4;
            rm = cpu.IR & 0x0F00;
            rm = ((int16_t) rm) >> 8;
            rd = cpu.IR & 0xF000;
            rd = ((int16_t)rd) >> 12;

            result = cpu.REG[rm]>>imm;

            cpu.REG[rd] = result & 0xFFFF;

            update_flags(&cpu, result);
            break;
         }
         case IEMAS_SHL: {
            imm = cpu.IR & 0x00F0;
            imm = ((int16_t) imm) >> 4;
            rm = cpu.IR & 0x0F00;
            rm = ((int16_t) rm) >> 8;
            rd = cpu.IR & 0xF000;
            rd = ((int16_t)rd) >> 12;

            result = cpu.REG[rm]<<imm;

            cpu.REG[rd] = result & 0xFFFF;

            update_flags(&cpu, result);
            break;
         }
         case IEMAS_CMP: {
            rn = cpu.IR & 0x00F0;
            rn = rn >> 4;
            rm = cpu.IR & 0x0F00;
            rm = rm >> 8;

            cpu.FLAGS = 0;

            if(cpu.REG[rm]<cpu.REG[rn])  {
               cpu.FLAGS |= FLAG_C;
            }
            if(cpu.REG[rm]==cpu.REG[rn])  {
               cpu.FLAGS |= FLAG_Z;
            }
            break;
         }
         case IEMAS_PUSH: {
            rn = cpu.IR & 0x00F0;
            rn = rn >> 4;

            cpu.REG[SP] --;
            cpu.MEM[cpu.REG[SP]] = cpu.REG[rn];
            break;
         case IEMAS_POP: {
            if(cpu.IR == 0xFFFF)  {
               break;
            }
            rd = cpu.IR & 0xF000;
            rd = rd >> 12;

            cpu.REG[rd] = cpu.MEM[cpu.REG[SP]];
            cpu.REG[SP] ++;
            break;
         }
      }
   }

      if(cpu.IR == 0xFFFF)  {
         loop = false;
      }
      if(is_breakpoint(cpu.REG[PC])) {
         printf("<== IEMAS Registers ==>\n");
         printf("PC = 0x%04X\n", cpu.REG[PC]);
         printf("IR = 0x%04X\n", cpu.IR);
         printf("FLAGS = 0x%02X\n", cpu.FLAGS);

         for (int i = 0; i < 16; i++) {
            printf("R%d = 0x%04X\n", i, cpu.REG[i]);
         }
        break;
      }
   }


   return 0;
}


