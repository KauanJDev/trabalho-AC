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
    IEMAS_JCO  = 0x1,

    IEMAS_LDR  = 0x2,
    IEMAS_STR  = 0x3,
    IEMAS_MOV  = 0x4,

    IEMAS_ADD  = 0x5,
    IEMAS_ADDI = 0x6,
    IEMAS_SUB  = 0x7,
    IEMAS_SUBI = 0x8,
    IEMAS_AND  = 0x9,
    IEMAS_OR   = 0xA,
    IEMAS_SHR  = 0xB,
    IEMAS_SHL  = 0xC,



    IEMAS_CMP  = 0xD,
    IEMAS_PUSH = 0xE,
    IEMAS_POP  = 0xF,

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

bool ioIn(IEMAS *cpu, uint16_t address, uint16_t rd)  {
   switch(address)  {
      case 0xF000:{
         scanf("%c", cpu->REG[rd]);
         printf("IN => %c\n", cpu->REG[rd]);
         return true;
         break;
      }
      case 0xF002:{
         scanf("%d", cpu->REG[rd]);
         printf("IR => %d\n", cpu->REG[rd]);
         return true;
         break;
      }
   }
   return false;
}

bool ioOut(IEMAS *cpu, uint16_t address, uint16_t rd)  {
   switch(address)  {
      case 0xF001:{
         printf("OUT => %c\n", cpu->REG[rd]);
         return true;
         break;
      }
      case 0xF003:{
         printf("OUT => %d\n", cpu->REG[rd]);
         return true;
         break;
      }
   }
   return false;
}

int main() {
   IEMAS cpu = {0}; 
   memset(cpu.MEM, 0, sizeof(cpu.MEM));

   cpu.REG[SP] = 0x2000;
   cpu.REG[PC] = 0x0000;

   scanf("%d", &num_bp);
   for (int i = 0; i < num_bp; i++) {
      scanf("%hx", &breakpoints[i]);
   }

   uint16_t address, buffer;
    while(scanf("%hX %hX%*[^\n]", &address, &buffer) == 2) {
      if(!address && !buffer) break;
		cpu.MEM[address] = buffer;
    }
   /*
   while(true)  {
      uint16_t address;
      uint16_t line;

      if (scanf("%hx %hx", &address, &line) != 2)  {
        break;
      }

      if((address == 0x0000 && line == 0x0000)) {
         break;
      }

      cpu.MEM[address] = line;
   }
   */

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
            uint16_t type = cpu.IR & 0xC000; // 1100 0000 0000 0000 
            type = type >> 14;
            imm = cpu.IR & 0x3FF0; // 0011 1111 1111 0000
            imm =  imm << 2;
            imm =  ((int16_t)imm) >> 6;
         
            if(type == 0)  {
               if(cpu.FLAGS & FLAG_Z)  {
                  cpu.REG[PC] += imm;
               }
            } else if(type == 1) {
               if(!(cpu.FLAGS & FLAG_Z))  {
                  cpu.REG[PC] += imm;
               }
            } else if(type == 2) {
               if(cpu.FLAGS & FLAG_C)  {
                  cpu.REG[PC] += imm;
               }
            } else  {
               if(!(cpu.FLAGS & FLAG_C))  {
                  cpu.REG[PC] += imm;
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


            if(!ioIn(&cpu, imm+rm, rd))  {
               cpu.REG[rd] = cpu.MEM[cpu.REG[rm]+imm];
            }
            break;
         }
         case IEMAS_STR:{
            rn = cpu.IR & 0x00F0;
            rn = rn >> 4;
            rm = cpu.IR & 0x0F00;
            rm = rm >> 8;
            imm = cpu.IR & 0xF000;
            imm = ((int16_t) imm) >> 12;


            if(!ioOut(&cpu, imm+rm, rd))  {
               cpu.MEM[cpu.REG[rm]+imm] = cpu.REG[rn];
            }
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
            rn = rn >> 4;
            rm = cpu.IR & 0x0F00;
            rm = rm >> 8;
            rd = cpu.IR & 0xF000;
            rd = rd >> 12;
            
            result = cpu.REG[rm]+cpu.REG[rn];

            cpu.REG[rd] = result & 0xFFFF;
             
            update_flags(&cpu, result);
            break;
         }
         case IEMAS_SUB: {
            rn = cpu.IR & 0x00F0;
            rn = rn >> 4;
            rm = cpu.IR & 0x0F00;
            rm = rm >> 8;
            rd = cpu.IR & 0xF000;
            rd = rd >> 12;

            result = cpu.REG[rm]-cpu.REG[rn];

            cpu.REG[rd] = result & 0xFFFF;

            update_flags(&cpu, result);
            break;
         }
         case IEMAS_AND: {
            rn = cpu.IR & 0x00F0;
            rn = rn >> 4;
            rm = cpu.IR & 0x0F00;
            rm = rm >> 8;
            rd = cpu.IR & 0xF000;
            rd = rd >> 12;

            result = cpu.REG[rm]&cpu.REG[rn];

            cpu.REG[rd] = result & 0xFFFF;

            update_flags(&cpu, result);
            break;
         }
         case IEMAS_OR: {
            rn = cpu.IR & 0x00F0;
            rn = rn >> 4;
            rm = cpu.IR & 0x0F00;
            rm = rm >> 8;
            rd = cpu.IR & 0xF000;
            rd = rd >> 12;

            result = cpu.REG[rm]|cpu.REG[rn];

            cpu.REG[rd] = result & 0xFFFF;

            update_flags(&cpu, result);
            break;
         }
         case IEMAS_ADDI: {
            imm = cpu.IR & 0x00F0;
            imm = ((int16_t) imm) >> 4;
            rm = cpu.IR & 0x0F00;
            rm =  rm >> 8;
            rd = cpu.IR & 0xF000;
            rd = rd >> 12;

            result = cpu.REG[rm]+imm;

            cpu.REG[rd] = result & 0xFFFF;

            update_flags(&cpu, result);
            break;
         }
         case IEMAS_SUBI: {
            imm = cpu.IR & 0x00F0;
            imm = ((int16_t) imm) >> 4;
            rm = cpu.IR & 0x0F00;
            rm =  rm >> 8;
            rd = cpu.IR & 0xF000;
            rd = rd >> 12;

            result = cpu.REG[rm]-imm;

            cpu.REG[rd] = result & 0xFFFF;

            update_flags(&cpu, result);
            break;
         }
         case IEMAS_SHR: {
            imm = cpu.IR & 0x00F0;
            imm = ((int16_t) imm) >> 4;
            rm = cpu.IR & 0x0F00;
            rm =  rm >> 8;
            rd = cpu.IR & 0xF000;
            rd = rd >> 12;

            result = cpu.REG[rm]>>imm;

            cpu.REG[rd] = result & 0xFFFF;

            update_flags(&cpu, result);
            break;
         }
         case IEMAS_SHL: {
            imm = cpu.IR & 0x00F0;
            imm = ((int16_t) imm) >> 4;
            rm = cpu.IR & 0x0F00;
            rm = rm >> 8;
            rd = cpu.IR & 0xF000;
            rd = rd >> 12;

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
         }
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
   

      if(cpu.IR == 0xFFFF)  {
         loop = false;
      }
      if(is_breakpoint(cpu.REG[PC])) {
         printf("PC = 0x%04X\n", cpu.REG[PC]);
         printf("IR = 0x%04X\n", cpu.IR);
         printf("FLAGS = 0x%02X\n", cpu.FLAGS);

         for (int i = 0; i < 16; i++) {
            printf("R%d = 0x%04X\n", i, cpu.REG[i]);
         }
      }
   }


   return 0;
}


