#include "linker.hpp"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void addSectionPlace(char* sectionName, uint32_t address){
  fixedPlaces[sectionName]=address;
}


void processSection(Elf32_Shdr *shdr, const char *sectionName, FILE *file, int fileIndex) {
    vector<uint8_t> sectionContent;
    sectionContent.resize(shdr->sh_size);
    Section newSection;
    newSection.name = strdup(sectionName);
    newSection.fixed = 0;
    newSection.written = 0;
    newSection.startAddr = 0;
    newSection.content = sectionContent;
    newSection.sectionHeader = shdr;
    newSection.fileIndex = fileIndex;

    fseek(file, shdr->sh_offset, SEEK_SET);
    fread(&newSection.content[0], sizeof(uint8_t), sectionContent.size(), file);
    sections.push_back(newSection);
}

void processElfFile(int fileIndex, const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        perror("Failed to open ELF file");
        exit(1);
    }

    Elf32_Ehdr elfHeader;
    fread(&elfHeader, 1, sizeof(elfHeader), file);

    fseek(file, elfHeader.e_shoff, SEEK_SET);

    Elf32_Shdr *sectionHeaders = (Elf32_Shdr *)malloc(elfHeader.e_shnum * sizeof(Elf32_Shdr));
    fread(sectionHeaders, elfHeader.e_shnum, sizeof(Elf32_Shdr), file);

    Elf32_Shdr *stringTableHeader = &sectionHeaders[elfHeader.e_shstrndx];

    uint8_t *strtab_content = (uint8_t*)malloc(stringTableHeader->sh_size);
    fseek(file, stringTableHeader->sh_offset, SEEK_SET);
    fread(strtab_content, 1, stringTableHeader->sh_size, file);

    for (int i = 0; i < elfHeader.e_shnum; i++) {
        fflush(stdout);
        Elf32_Shdr *sectionHeader = &sectionHeaders[i];
        const char *sectionName = (char*)strtab_content + sectionHeader->sh_name;
        if(sectionHeader->sh_type == 1){
            processSection(sectionHeader, sectionName, file, fileIndex);
        }
    }
    free(sectionHeaders);
    free(strtab_content);
    fclose(file);
}

void checkForOverlappingAddresses(){
    for(int i = 0; i<sections.size(); i++){
        for(int j = i+1; j<sections.size(); j++){
            uint32_t s1End = sections[i].startAddr + sections[i].content.size();
            uint32_t s2End = sections[j].startAddr + sections[j].content.size();
            uint32_t s1Start= sections[i].startAddr;
            uint32_t s2Start = sections[j].startAddr;
            if((s1End > s2Start && s1End < s2End)\
            || \
            (s2End > s1Start && s2End < s1End)){
                perror("Overlapping addresses!");
                exit(-1);
            }
        }
    }
}

uint32_t getNextFreeAddress(){
    uint32_t maxAddr = 0;
    for(int i = 0; i<sections.size(); i++){
        if(sections[i].fixed==1){
            maxAddr = max((uint32_t)(sections[i].startAddr + sections[i].content.size()),maxAddr);
        }
    }
    for(int i = 0; i<sections.size(); i++){
        maxAddr = max((uint32_t)(sections[i].startAddr + sections[i].content.size()),maxAddr);
    }
    return maxAddr;
}

void fixStartingAddressesBySectionName(char* sectionName){
    uint32_t currAddr = getNextFreeAddress();
    for(int i = 0; i<sections.size(); i++){
        if(strcmp(sections[i].name,sectionName)==0){
            if(sections[i].fixed){
                return;
            }
            sections[i].startAddr = currAddr;
            currAddr += sections[i].content.size();
        }
    }
}

void fixStartingAddressesWithFixedPlace(char* sectionName, uint32_t currAddr){
    for(int i = 0; i<sections.size(); i++){
        if(strcmp(sections[i].name,sectionName)==0){
            sections[i].startAddr = currAddr;
            sections[i].fixed = 1;
            currAddr += sections[i].content.size();
        }
    }
}


void fixStartingAddresses(){
    int numOfDisctinctNames = 0;
    vector<char*> sectionNames;
    for(int i = 0; i<sections.size(); i++){
        char* sectionName = sections[i].name;
        int found = 0;
        for(int j=0;j<sectionNames.size();j++){
            if(strcmp(sectionNames[j],sectionName)==0){
                found = 1;
                break;
            }
        }
        if(found==0){
            sectionNames.push_back(sectionName);
        }
    }
    for(auto i = fixedPlaces.begin(); i!= fixedPlaces.end(); i++){
        fixStartingAddressesWithFixedPlace(i->first,i->second);
    }

    for(int i = 0;i< sectionNames.size();i++){
        fixStartingAddressesBySectionName(sectionNames[i]);
    }
}

vector<char*> inputFiles;

void mapContent(){
    int debug = 0;
    for (int i = 0; i < inputFiles.size(); i++)
    {
        processElfFile(i,inputFiles[i]);
    }
    fixStartingAddresses();
    checkForOverlappingAddresses();

    if(debug){
        for(int i=0;i<sections.size();i++){
            Section section = sections[i];
        }
    }
}

void determination(int isRelocatable = 0) {
    int debug = 0;
    for (int i = 0; i < inputFiles.size(); i++) {
        const char *filename = inputFiles[i];

        FILE *file = fopen(filename, "rb");
        if (file == NULL) {
            perror("Failed to open ELF file");
            exit(1);
        }

        Elf32_Ehdr elfHeader;
        fread(&elfHeader, 1, sizeof(elfHeader), file);

        fseek(file, elfHeader.e_shoff, SEEK_SET);
        Elf32_Shdr *sectionHeaders = (Elf32_Shdr *)malloc(elfHeader.e_shnum * sizeof(Elf32_Shdr));
        fread(sectionHeaders, elfHeader.e_shnum, sizeof(Elf32_Shdr), file);

        Elf32_Shdr *shstrtabHeader = &sectionHeaders[1];
        uint8_t *shstrtab_content = (uint8_t *)malloc(shstrtabHeader->sh_size);
        fseek(file, shstrtabHeader->sh_offset, SEEK_SET);
        fread(shstrtab_content, 1, shstrtabHeader->sh_size, file);

        Elf32_Shdr *strtabHeader = &sectionHeaders[2];
        uint8_t *strtab_content = (uint8_t *)malloc(strtabHeader->sh_size);
        fseek(file, strtabHeader->sh_offset, SEEK_SET);
        fread(strtab_content, 1, strtabHeader->sh_size, file);

        Elf32_Shdr *symtabHeader = &sectionHeaders[3];
        int numSymbols = symtabHeader->sh_size / symtabHeader->sh_entsize;
        fseek(file, symtabHeader->sh_offset, SEEK_SET);
        Elf32_Sym *symbolTable = (Elf32_Sym *)malloc(symtabHeader->sh_size);
        fread(symbolTable, 1, symtabHeader->sh_size, file);

        for (int k = 0; k < numSymbols; k++) {
            Elf32_Sym *symbol = &symbolTable[k];
            const char *symbolName = (char *)strtab_content + symbol->st_name;

            if (ELF32_ST_TYPE(symbol->st_info) == STT_NOTYPE && symbol->st_shndx != SHN_UNDEF && ELF32_ST_BIND(symbol->st_info) == STB_GLOBAL) {

                Elf32_Sym *symSection = &symbolTable[symbol->st_shndx];
                const char *symSectionName = (char *)(strtab_content + symSection->st_name);

                uint32_t sectionStartAddr = 0;
                bool found = false;
                for (int l = 0; l < sections.size(); l++) {
                    if (strcmp(sections[l].name, symSectionName) == 0 && sections[l].fileIndex == i) {
                        sectionStartAddr = sections[l].startAddr;
                        found = true;
                        break;
                    }
                }

                if (found) {
                    uint32_t symbolValue = sectionStartAddr + symbol->st_value;
                    char *symbolNameCopy = strdup(symbolName);
                    if(symbolAddresses[symbolNameCopy]){
                        fprintf(stderr,"Multiple definition of symbol: %s\n",symbolName);
                        exit(-1);
                    }
                    symbolAddresses[symbolNameCopy] = symbolValue;
                } else {
                    fprintf(stderr, "Section %s not found for symbol %s\n", symSectionName, symbolName);
                    exit(-1);
                }
            }
        }        
        free(symbolTable);
        free(shstrtab_content);
        free(strtab_content);
        free(sectionHeaders);
        fclose(file);
    }
    for(auto i = symbolAddresses.begin(); i!= symbolAddresses.end(); i++){
        char* symbolName = i->first;
        uint32_t symbolAddress = i->second;
        if(debug)printf("Symbol: %s, Address: 0x%X\n", symbolName, symbolAddress);
    }
}



void resolution() {
    for (int i = 0; i < inputFiles.size(); i++) {
        const char *filename = inputFiles[i];

        FILE *file = fopen(filename, "rb");
        if (file == NULL) {
            perror("Failed to open ELF file");
            exit(1);
        }

        Elf32_Ehdr elfHeader;
        fread(&elfHeader, 1, sizeof(elfHeader), file);

        fseek(file, elfHeader.e_shoff, SEEK_SET);
        Elf32_Shdr *sectionHeaders = (Elf32_Shdr *)malloc(elfHeader.e_shnum * sizeof(Elf32_Shdr));
        fread(sectionHeaders, elfHeader.e_shnum, sizeof(Elf32_Shdr), file);

        Elf32_Shdr *shstrtabHeader = &sectionHeaders[1];
        uint8_t *shstrtab_content = (uint8_t *)malloc(shstrtabHeader->sh_size);
        fseek(file, shstrtabHeader->sh_offset, SEEK_SET);
        fread(shstrtab_content, 1, shstrtabHeader->sh_size, file);

        for (int idx = 0; idx < elfHeader.e_shnum; idx++) {
            Elf32_Shdr *sectionHeader = &sectionHeaders[idx];
            const char *sectionName = (char *)shstrtab_content + sectionHeader->sh_name;

            if (sectionHeader->sh_type == SHT_RELA) {

                int numRelocs = sectionHeader->sh_size / sectionHeader->sh_entsize;

                fseek(file, sectionHeader->sh_offset, SEEK_SET);
                Elf32_Rela *relEntries = (Elf32_Rela *)malloc(sectionHeader->sh_size);
                fread(relEntries, 1, sectionHeader->sh_size, file);

                int symtabIndex = sectionHeader->sh_link;
                Elf32_Shdr *symtabHeader = &sectionHeaders[symtabIndex];
                Elf32_Shdr *strtabHeader = &sectionHeaders[2];
                uint8_t *strtab_content = (uint8_t *)malloc(strtabHeader->sh_size);
                fseek(file, strtabHeader->sh_offset, SEEK_SET);
                fread(strtab_content, 1, strtabHeader->sh_size, file);

                int numSymbols = symtabHeader->sh_size / symtabHeader->sh_entsize;
                fseek(file, symtabHeader->sh_offset, SEEK_SET);
                Elf32_Sym *symbolTable = (Elf32_Sym *)malloc(symtabHeader->sh_size);
                fread(symbolTable, 1, symtabHeader->sh_size, file);

                int targetSectionIndex = sectionHeader->sh_info;
                Elf32_Shdr *targetSectionHeader = &sectionHeaders[targetSectionIndex];
                const char *targetSectionName = (char *)shstrtab_content + targetSectionHeader->sh_name;
                Section *targetSection = nullptr;
                for (int s = 0; s < sections.size(); s++) {
                    if (strcmp(sections[s].name, targetSectionName) == 0 && sections[s].fileIndex == i) {
                        targetSection = &sections[s];
                        break;
                    }
                }
                if (!targetSection) {
                    fprintf(stderr, "Target section %s not found for relocation\n", targetSectionName);
                    exit(-1);
                }

                for (int r = 0; r < numRelocs; r++) {
                    Elf32_Rela *rel = &relEntries[r];
                    uint32_t offset = rel->r_offset;
                    uint32_t info = rel->r_info;
                    uint32_t addend = rel->r_addend;
                    uint32_t symIndex = ELF32_R_SYM(info);
                    uint32_t relType = ELF32_R_TYPE(info);

                    Elf32_Sym *symbol = &symbolTable[symIndex];
                    const char *symbolName = (char *)strtab_content + symbol->st_name;

                    uint32_t symbolAddr = 0;
                    bool symbolFound = false;
                    
                    for (auto it = symbolAddresses.begin(); it != symbolAddresses.end(); ++it) {
                        if (strcmp(it->first, symbolName) == 0) {
                            symbolAddr = it->second;
                            symbolFound = true;
                            break;
                        }
                    }
                    if (!symbolFound){
                        bool foundSection = false;
                        for (int s = 0; s < sections.size(); s++) {
                            if (strcmp(sections[s].name, symbolName) == 0 && sections[s].fileIndex == i) {
                                symbolAddr = sections[s].startAddr;
                                foundSection = true;
                                break;
                            }
                        }
                        if (!foundSection) {
                            fprintf(stderr, "Symbol %s is not defined\n", symbolName);
                            exit(-1);
                        }
                    }

                    uint32_t relocationValue = 0;
                    switch (relType) {
                        case R_X86_64_32:
                            relocationValue = symbolAddr + addend;
                            break;
                        default:
                            fprintf(stderr, "Unsupported relocation type %d\n", relType);
                            exit(-1);
                    }
                    uint32_t relOffset = offset;
                    if (relOffset + 4 > targetSection->content.size()) {
                        fprintf(stderr, "Relocation offset out of bounds\n");
                        exit(-1);
                    }
                    targetSection->content[relOffset] = relocationValue & 0xFF;
                    targetSection->content[relOffset + 1] = (relocationValue >> 8) & 0xFF;
                    targetSection->content[relOffset + 2] = (relocationValue >> 16) & 0xFF;
                    targetSection->content[relOffset + 3] = (relocationValue >> 24) & 0xFF;
                }

                free(relEntries);
                free(symbolTable);
                free(strtab_content);
            }
        }

        free(sectionHeaders);
        free(shstrtab_content);
        fclose(file);
    }
}


void writeToHexFile(char* outputFileName){

    FILE* outFile = fopen(outputFileName, "w");
    if (outFile == NULL) {
        perror("Failed to open output file");
        exit(1);
    }
    std::map<uint32_t, uint8_t> memoryContent;

    for (size_t i = 0; i < sections.size(); i++) {
        Section& section = sections[i];

        uint32_t startAddr = section.startAddr;
        std::vector<uint8_t>& content = section.content;

        for (size_t offset = 0; offset < content.size(); offset++) {
            uint32_t addr = startAddr + offset;
            uint8_t byte = content[offset];
            memoryContent[addr] = byte;
        }
    }

    if (memoryContent.empty()) {
        fprintf(stderr, "No memory content to write.\n");
        fclose(outFile);
        exit(-1);
    }

    std::map<uint32_t, std::vector<std::string>> lines;

    for (auto it = memoryContent.begin(); it != memoryContent.end(); ++it) {
        uint32_t addr = it->first;
        uint8_t byte = it->second;

        uint32_t lineStartAddr = addr & ~0x7;

        int offset = addr & 0x7;

        if (lines[lineStartAddr].empty()) {
            lines[lineStartAddr] = std::vector<std::string>(8, "   ");
        }
        char byteStr[4];
        sprintf(byteStr, "%02X ", byte);
        lines[lineStartAddr][offset] = std::string(byteStr);
    }

    for (auto it = lines.begin(); it != lines.end(); ++it) {
        uint32_t lineStartAddr = it->first;
        std::vector<std::string>& byteStrs = it->second;

        fprintf(outFile, "%04X: ", lineStartAddr);
        for (int i = 0; i < 8; i++) {
            fprintf(outFile, "%s", byteStrs[i].c_str());
        }
        fprintf(outFile, "\n");
    }
    fclose(outFile);
}

int main(int argc, char** argv){
  int i = 1;
  int next = 0;
  char* prev;
  char* output_file;
  int hex = 0;
  
  while(i < argc){
      char* argument = argv[i];
      char* sectionName = NULL;
      char* address = NULL;
      if(next == 1){
        output_file = argv[i];
        next = 0;
        i++;
        continue;
      }

      if(strcmp(argv[i],"-o") == 0 && next == 0){
          next = 1;
      } else if(strncmp(argument, "-place=", 7) == 0) {
          const char *atSign = strchr(argument, '@');
          if (atSign) {
              size_t sectionNameLen = atSign - (argument + 7);
              sectionName = (char *)malloc(sectionNameLen + 1);
              strncpy(sectionName, argument + 7, sectionNameLen);
              sectionName[sectionNameLen] = '\0';
              address = strdup(atSign + 1);
          } else {
              sectionName = NULL;
              address = NULL;
          }
          addSectionPlace(sectionName, (uint32_t)strtoul(address, NULL, 16));
      } else if(strcmp(argument, "-hex") == 0){
          hex = 1;
      } else if(strlen(argument) > 2 && strcmp(argument + strlen(argument) - 2, ".o") == 0) {
            inputFiles.push_back(strdup(argument));
      }
      i++;
  }

  if(hex == 0){perror("error, no hex attribute");return -1;}
  mapContent();
  determination();
  resolution();
  writeToHexFile(output_file);
}