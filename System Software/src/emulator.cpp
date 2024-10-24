#include "emulator.hpp"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void finish(){
  printf("-------------------------------------------------------------------------\n");
  printf("Emulated processor executed halt instruction\n");
  printf("Emulated processor state:\n");

  for (int i = 0; i < 16; i++) {
    printf("r%d=0x%08X\t", i, r[i]);
    if ((i + 1) % 4 == 0) {
      printf("\n");
    }
  }
}

void finish2(){
  for (int i = 0; i < 16; i++) {
    printf("r%d=0x%08X\t", i, r[i]);
    if ((i + 1) % 4 == 0) {
      printf("\n");
    }
  }
  for(int i=0;i<3;i++){
    printf("csr%d=0x%08X\t",i,csr[i]);
  }
  printf("\n");
}

void initializeData(){
  r.resize(16);
  for (int i = 0; i < 16; i++)
  {
    r[i]=0x00000000;
  }
  r[15]=0x40000000;
  //status, handler, cause
  csr.resize(3);
  for (int i = 0; i < 3; i++)
  {
    csr[i]=0x00000000;
  }
  irq = 0;
}

void loadMemory(const char* fileName){
  FILE* inFile = fopen(fileName, "r");
  if (inFile == NULL) {
    perror("Failed to open memory initialization file");
    exit(1);
  }

  char line[256];
  while (fgets(line, sizeof(line), inFile)) {
    char* colonPos = strchr(line, ':');
    if (colonPos == NULL) continue;
    uint32_t address;
    if (sscanf(line, "%x:", &address) != 1) continue;
    char* byteStr = colonPos + 1;

    for (int i = 0; i < 8; i++) {
      while (*byteStr == ' ' || *byteStr == '\t') byteStr++;
      if (*byteStr == '\0' || *byteStr == '\n') break;
      unsigned int byteValue;
      if (sscanf(byteStr, "%2x", &byteValue) != 1) {
        break;
      }
      mem[address + i] = (uint8_t)byteValue;
      byteStr += 2;
    }
  }

  fclose(inFile);
}

uint32_t read(uint32_t address) {
  uint32_t value = 0;
  for (int i = 0; i < 4; ++i) {
    uint8_t byte = mem[address + i];
    value |= static_cast<uint32_t>(byte) << (8 * i);
  }
  return value;
}
void write(uint32_t address, uint32_t value) {
  for (int i = 0; i < 4; ++i) {
    uint8_t byte = (value >> (8 * i)) & 0xFF;
    mem[address + i] = byte;
  }
}

void push(uint32_t val){
  r[14] = r[14] - 4;
  write(r[14],val);
}

uint32_t pop(){
  uint32_t val = read(r[14]);
  r[14]=r[14]+4;
  return val;
}

void emulate(){
  int debug = 0;
   while (true) {
    uint32_t instruction = mem[r[15]] | (mem[r[15] + 1] << 8) | (mem[r[15] + 2] << 16) | (mem[r[15] + 3] << 24);
    uint8_t oc = (instruction >> 28) & 0xF;
    uint8_t mod = (instruction >> 24) & 0xF;
    uint8_t a = (instruction >> 20) & 0xF;
    uint8_t b = (instruction >> 16) & 0xF;
    uint8_t c = (instruction >> 12) & 0xF;
    int16_t disp = instruction & 0xFFF;

    r[15] += 4;
    if (disp & 0x800) { //if negative, extends 1s
        disp = disp | 0xF000;
    }

    switch (oc) {
        case 0x00: //HALT
          if(debug)printf("zaustavljanje procesora...\n");
          return;
        case 0x01:
          if(debug)printf("softverski prekid\n");
          irq |=4;
          break;
        case 0x02:
          if(debug)printf("poziv potprograma\n");
          push(r[15]);
          if(mod == 0b0000){
            r[15] = r[a] + r[b] + disp;
          }else
          if(mod == 0b0001){
            r[15] = read(r[a] + r[b] + disp);
          }
          break;
        case 0x03:
          if(debug)printf("skok\n");
          if(mod == 0b0000){
            r[15] = r[a] + disp;
          }else
          if(mod == 0b0001){
            if(r[b] == r[c])r[15] = r[a] + disp;
          }else
          if(mod == 0b0010){
            if(r[b] != r[c])r[15] = r[a] + disp;
          }else
          if(mod == 0b0011){
            if(static_cast<int32_t>(r[b]) > static_cast<int32_t>(r[c]))r[15] = r[a] + disp;
          }else
          if(mod == 0b1000){
            r[15] = read(r[a]+disp);
          }else
          if(mod == 0b1001){
            if(r[b] == r[c])r[15] = read(r[a] + disp);
          }else
          if(mod == 0b1010){
            if(r[b] != r[c])r[15] = read(r[a] + disp);
          }else
          if(mod == 0b1011){
            if(static_cast<int32_t>(r[b]) > static_cast<int32_t>(r[c]))r[15] = read(r[a] + disp);
          }
          break;
        case 0x04:
          if(debug)printf("atomicna zamena vrednosti\n");
          uint8_t temp;
          temp = r[b];
          r[b] = r[c];
          r[c] = temp;
          break;
        case 0x05:
          if(debug)printf("artitmeticka operacija\n");
          if(mod == 0b0000){
            r[a] = r[b]+r[c];
          }else
          if(mod == 0b0001){
            r[a] = r[b]-r[c];
          }else
          if(mod == 0b0010){
            r[a] = r[b]*r[c];
          }else
          if(mod == 0b0011){
            r[a] = r[b]/r[c];
          }
          break;
        case 0x06:
          if(debug)printf("logicka operacija\n");
          if(mod == 0b0000){
            r[a] = ~r[b];
          }else
          if(mod == 0b0001){
            r[a] = r[b] & r[c];
          }else
          if(mod == 0b0010){
            r[a] = r[b] | r[c];
          }else
          if(mod == 0b0011){
            r[a] = r[b] ^ r[c];
          }
          break;
        case 0x07:
          if(debug)printf("pomeracka operacija\n");
          if(mod == 0b0000){
            r[a] = (r[b] << r[c]);
          }else
          if(mod == 0b0001){
            r[a] = (r[b] >> r[c]);
          }
          break;
        case 0x08:
          if(debug)printf("smestanje podatka\n");
          if(mod == 0b0000){
            write(r[a]+r[b]+disp,r[c]);
          }else
          if(mod == 0b0010){
            write(read(r[a]+r[b]+disp),r[c]);
          }else
          if(mod == 0b0001){
            r[a] = r[a]+disp;
            write(r[a],r[c]);
          }
          break;
        case 0x09:
          if(debug)printf("ucitavanje podatka: 0x%X\n",instruction);
          if(mod == 0b0000){
            r[a] = csr[b];
          }else
          if(mod == 0b0001){
            r[a] = r[b] + disp;
          }else
          if(mod == 0b0010){
            uint32_t zbir = r[b] + r[c] + disp;
            r[a] = read(r[b]+r[c]+disp);
          }else
          if(mod == 0b0011){
            r[a] = read(r[b]);
            r[b] = r[b] + disp;
          }else
          if(mod == 0b0100){
            csr[a] = r[b];
          }else
          if(mod == 0b0101){
            csr[a] = r[b] | disp;
          }else
          if(mod == 0b0110){
            csr[a] = read(r[b]+r[c]+disp);
          }else
          if(mod == 0b0111){
            csr[a] = read(r[b]);
            r[b] = r[b] + disp;
          }
          break;
        default:
          if(debug)printf("los op kod\n");
          irq|=1;
          break;
    }
    if(irq!=0 && (~csr[0])&4){
      push(r[15]);//push pc
      push(csr[0]);//push status
      if(irq&4){
        csr[2]=4;
        irq=irq&(~4);
      }else if(irq&1){
        csr[2]=1;
        irq=irq&(~1);
      }
      csr[0] = csr[0]|(0x4);
      r[15] = csr[1];
    }
    }
}

int main(int argc, char** argv){
  initializeData();

  if (argc < 2) {
    fprintf(stderr, "Usage: ./emulator <name_of_hex_file>\n");
    exit(1);
  }
  const char* memoryFile = argv[1];
  loadMemory(memoryFile);
  emulate();
  finish();
  return 0;
}