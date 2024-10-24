#include <stdio.h>
#include <string.h>
#include "assembler.h"
#include "lex.yy.c"

#define MAX_LINE_LENGTH 1024
#define SHSTR_INDEX 1
#define STR_INDEX 2
#define SYM_INDEX 3
char line_buffer[MAX_LINE_LENGTH];
FILE *input_file;
ParsedLine parsed_line;

ElfFile elfFile;
ElfHelper elfHelper;

char* readNextLine() {
    if (fgets(line_buffer, MAX_LINE_LENGTH, input_file) != NULL) {
        return line_buffer;
    } else {
        return NULL;
    }
}

int getOffset(char *needle, char *haystack, int haystackLen)
{
    int needleLen = strlen(needle);
    char *search = haystack;
    int searchLen = haystackLen - needleLen + 1;
    for (; searchLen-- > 0; search++)
    {
        if (!strcmp(search, needle))
        {
            return search - haystack;
        }
    }
    return 0;
}

void freeUsedMemory(){
    for (int i = 0; i < parsed_line.arg_count; i++) {
        free(parsed_line.arguments[i]);
        parsed_line.arguments[i] = NULL;
    }
    if (parsed_line.arguments) {
        free(parsed_line.arguments);
        parsed_line.arguments = NULL;
    }
    if (parsed_line.isLiteral){
        free(parsed_line.isLiteral);
        parsed_line.isLiteral= NULL;
    }
    if (parsed_line.label) {
        free(parsed_line.label);
    }
    if (parsed_line.mnemonic) {
        free(parsed_line.mnemonic);
        parsed_line.mnemonic = NULL;
    }
    parsed_line.arg_count = 0;
    parsed_line.empty = 0;
    parsed_line.info = -1;
    parsed_line.label = NULL;

}

void initializeElfHeader(){
    Elf32_Ehdr tempHeader = {
        .e_ident = {/* Magic number and other info */
                    /* [0] EI_MAG        */ 0x7F, 'E', 'L', 'F',
                    /* [4] EI_CLASS      */ ELFCLASS32,
                    /* [5] EI_DATA       */ ELFDATA2LSB,
                    /* [6] EI_VERSION    */ EV_CURRENT,
                    /* [7] EI_OSABI      */ ELFOSABI_SYSV,
                    /* [8] EI_ABIVERSION */ 0,
                    /* [9-15] EI_PAD     */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
                    },
        .e_type = ET_REL,                                                  /* Object file type */
        .e_machine = EM_X86_64,                                            /* Required architecture */
        .e_version = EV_CURRENT,                                           /* Object file version */
        .e_entry = 0,                                                      /* Entry point virtual address */
        .e_phoff = 0,                                                      /* Program header table file offset */
        .e_shoff = 0,                                                      /* Section header table file offset */ 
        .e_flags = 0,                                                      /* Processor-specific flags */
        .e_ehsize = sizeof(Elf32_Ehdr),                                    /* ELF header size in bytes */
        .e_phentsize = 0,                                                  /* Program header table entry size */
        .e_phnum = 0,                                                      /* Program header table entry count */
        .e_shentsize = sizeof(Elf32_Shdr),                                 /* Section header table entry size */
        .e_shnum = 4,                                                      /* Section header table entry count */
        .e_shstrndx = 1                                                    /* Section header string table index */ 
    };
    elfFile.elfHeader = tempHeader;
    elfHelper.numOfSections=4;
    elfFile.sections = malloc(elfHelper.numOfSections*sizeof(uint8_t*));
    elfHelper.sizes = malloc(4*sizeof(size_t));//undfnd, shstr_tab, str_tab, symtab
    elfFile.sections[0]=NULL;
    elfHelper.sizes[0]=0;
    elfHelper.activeSection = -1;

    elfHelper.literalSymbolTables = malloc(elfHelper.numOfSections * sizeof(SectionLiteralSymbolTable));
    for (size_t i = 0; i < elfHelper.numOfSections; i++) {
        elfHelper.literalSymbolTables[i].entries = NULL;
        elfHelper.literalSymbolTables[i].count = 0;
    }
    elfHelper.relIndex = malloc(elfHelper.numOfSections * sizeof(int));
    for (size_t i = 0; i < elfHelper.numOfSections; i++) {
        elfHelper.relIndex[i] = -1;
    }
    elfHelper.wordEntries = malloc(elfHelper.numOfSections * sizeof(WordEntries));
    for (size_t i = 0; i < elfHelper.numOfSections; i++) {
        elfHelper.wordEntries[i].entries = NULL;
        elfHelper.wordEntries[i].count = 0;
    }
}

void initializeShstrTable(){

    elfFile.sections[1] = malloc(27*sizeof(uint8_t));
    elfHelper.sizes[1]=27;
    if (elfFile.sections[1] == NULL) {
        perror("Failed to allocate memory for shstrtab");
        exit(1);
    }
    memcpy(elfFile.sections[1], "\0.symtab\0.strtab\0.shstrtab\0",27);
}

void initializeStrTable(){
    size_t numOfSections = 2;

    elfFile.sections[numOfSections] = malloc(1*sizeof(uint8_t));
    elfHelper.sizes[numOfSections]=1;
    if (elfFile.sections[numOfSections] == NULL) {
        perror("Failed to allocate memory for shstrtab");
        exit(1);
    }
    strcpy((char*)elfFile.sections[numOfSections], "\0");//null character at the start, used by the undefined symbol
}

void initializeSymbolTable(){
    size_t numOfSections = 3;

    elfFile.sections[numOfSections] = malloc(1*sizeof(Elf32_Sym));
    elfHelper.sizes[numOfSections]=sizeof(Elf32_Sym);

    if (elfFile.sections[numOfSections] == NULL) {
        perror("Failed to allocate memory for section header table");
        exit(1);
    }
    Elf32_Sym symbolUndef = {
        .st_name = 0,
        .st_info = ELF32_ST_INFO(STB_LOCAL, STT_NOTYPE),
        .st_other = STV_DEFAULT,
        .st_shndx = SHN_UNDEF, // *UND*
        .st_value = 0x00000000u,
        .st_size = 0,
    };
    memcpy(elfFile.sections[numOfSections], &symbolUndef, sizeof(Elf32_Sym));
}

void initializeSectionHeaderTable(){
    size_t numOfSections = elfHelper.numOfSections;
    elfFile.sectionHeaderTable = malloc((numOfSections)*sizeof(Elf32_Shdr));
    if (elfFile.sectionHeaderTable == NULL) {
        perror("Failed to allocate memory for section header table");
        exit(1);
    }
    Elf32_Shdr undfndSection = {
        .sh_name = 0,
        .sh_type = 0,
        .sh_flags = 0,
        .sh_addr = 0x00000000u,
        .sh_offset = 0,
        .sh_size = 0,
        .sh_link = 0,
        .sh_info = 0,
        .sh_addralign = 0,
        .sh_entsize = 0,
    };
    Elf32_Shdr shstrTable = {
        .sh_name = getOffset(".shstrtab", elfFile.sections[SHSTR_INDEX], elfHelper.sizes[SHSTR_INDEX]),
        .sh_type = SHT_STRTAB,
        .sh_flags = 0,
        .sh_addr = 0x00000000u,
        .sh_offset = 0,
        .sh_size = 0x1b,
        .sh_link = 0,
        .sh_info = 0,
        .sh_addralign = 1,
        .sh_entsize = 0,
    };
    Elf32_Shdr strTable = {
        .sh_name = getOffset(".strtab", elfFile.sections[SHSTR_INDEX], elfHelper.sizes[SHSTR_INDEX]),
        .sh_type = SHT_STRTAB,
        .sh_flags = 0,
        .sh_addr = 0x00000000u,
        .sh_offset = 0,
        .sh_size = 0x34,
        .sh_link = 0,
        .sh_info = 0,
        .sh_addralign = 1,
        .sh_entsize = 0,
    };
    Elf32_Shdr symTable = {
        .sh_name = getOffset(".symtab", elfFile.sections[SHSTR_INDEX], elfHelper.sizes[SHSTR_INDEX]),
        .sh_type = SHT_SYMTAB,
        .sh_flags = 0,
        .sh_addr = 0x00000000u,
        .sh_offset = 0,
        .sh_size = 0xc0,
        .sh_link = 2,      // section header index for .strtab
        .sh_info = 0, // one greater than the symbol table index of the last local symbol 
        .sh_addralign = 8,
        .sh_entsize = sizeof(Elf32_Sym),
    };
    elfFile.sectionHeaderTable[0] = undfndSection;
    elfFile.sectionHeaderTable[1] = shstrTable;
    elfFile.sectionHeaderTable[2] = strTable;
    elfFile.sectionHeaderTable[3] = symTable;
}

void initializeStructures(){
    initializeElfHeader();
    initializeShstrTable();
    initializeStrTable();
    initializeSymbolTable();
    initializeSectionHeaderTable();
}

int getSymbolIndex(const char* symbolName) {
    for (int i = 0; i < elfHelper.sizes[SYM_INDEX] / sizeof(Elf32_Sym); i++) {
        Elf32_Sym* sym = (Elf32_Sym*)(elfFile.sections[SYM_INDEX] + i * sizeof(Elf32_Sym));
        const char* name = (char*)(elfFile.sections[STR_INDEX] + sym->st_name);

        if (strcmp(name, symbolName) == 0) {
            return i;
        }
    }
    return -1;
}

Elf32_Sym* getSymbolByIndex(int index) {
    if (index < 0 || index >= (elfHelper.sizes[SYM_INDEX] / sizeof(Elf32_Sym))) {
        return NULL;
    }
    Elf32_Sym* sym = (Elf32_Sym*)(elfFile.sections[SYM_INDEX] + index * sizeof(Elf32_Sym));
    return sym;
}

int getSectionByName(char* sectionName){
    int sectionIndex = -1;
    for (int i = 1; i < elfHelper.numOfSections; i++) {
        if (strcmp(elfFile.sections[SHSTR_INDEX] + elfFile.sectionHeaderTable[i].sh_name, sectionName) == 0) {
            sectionIndex = i;
            break;
        }
    }
    return sectionIndex;
}

char* getNameOfSection(int sectionIndex) {
    if (sectionIndex < 0 || sectionIndex >= elfHelper.numOfSections) {
        return NULL;
    }
    uint32_t nameOffset = elfFile.sectionHeaderTable[sectionIndex].sh_name;
    return (char *)(elfFile.sections[SHSTR_INDEX] + nameOffset);
}

int isGlobalSymbol(int symIndex) {
    for (int i = 0; i < elfHelper.numGlobal; i++) {
        if (elfHelper.globalSymbols[i] == symIndex) {
            return 1; 
        }
    }
    return 0;
}

void addStringToStrTab(char* symbolName){
    int symbolNameLength = strlen(symbolName) + 1;
    uint8_t* temp = realloc(elfFile.sections[STR_INDEX], elfHelper.sizes[STR_INDEX] + symbolNameLength);
    if (temp == NULL) {
        perror("Failed to resize strtab");
        exit(1);
    }
    elfFile.sections[STR_INDEX] = temp;
    strcpy((char*)(elfFile.sections[STR_INDEX] + elfHelper.sizes[STR_INDEX]), symbolName);
    elfHelper.sizes[STR_INDEX] += symbolNameLength;
}

void addStringToShstrTab(char* sectionName) {
    int length = strlen(sectionName) + 1;
    uint8_t* temp = realloc(elfFile.sections[SHSTR_INDEX], elfHelper.sizes[SHSTR_INDEX] + length);
    if (temp == NULL) {
        perror("Failed to resize shstrtab");
        exit(1);
    }
    elfFile.sections[SHSTR_INDEX] = temp;
    strcpy((char*)(elfFile.sections[SHSTR_INDEX] + elfHelper.sizes[SHSTR_INDEX]), sectionName);
    elfHelper.sizes[SHSTR_INDEX] += length;
}

void addSymbolToSymtab(char* symbolName, int type) {
    Elf32_Sym newSymbol;
    int symbolIndex = getSymbolIndex(symbolName);
    if (symbolIndex != -1) {
        Elf32_Sym* existingSymbol = (Elf32_Sym*)(elfFile.sections[SYM_INDEX] + symbolIndex * sizeof(Elf32_Sym));
        
        if (type == STB_LOCAL) {
            existingSymbol->st_shndx = getSymbolIndex(getNameOfSection(elfHelper.activeSection));
            existingSymbol->st_value = elfHelper.sizes[elfHelper.activeSection];
        } 

    } else {
        if (getOffset(symbolName, elfFile.sections[STR_INDEX], elfHelper.sizes[STR_INDEX]) == 0) {
            addStringToStrTab(symbolName);
        }

        newSymbol.st_name = getOffset(symbolName, elfFile.sections[STR_INDEX], elfHelper.sizes[STR_INDEX]);
        newSymbol.st_other = STV_DEFAULT;
        newSymbol.st_size = 0;

        if (type == STB_LOCAL) {
            newSymbol.st_info = ELF32_ST_INFO(STB_LOCAL, STT_NOTYPE);
            newSymbol.st_shndx = getSymbolIndex(getNameOfSection(elfHelper.activeSection));
            newSymbol.st_value = elfHelper.sizes[elfHelper.activeSection];
        } else if(type ==STB_GLOBAL){
            newSymbol.st_info = ELF32_ST_INFO(STB_GLOBAL, STT_NOTYPE);
            newSymbol.st_shndx = SHN_UNDEF;
            newSymbol.st_value = 0;
        }else{ //symbol is a section
            newSymbol.st_info = ELF32_ST_INFO(STB_LOCAL, STT_SECTION);
            newSymbol.st_shndx = elfHelper.sizes[SYM_INDEX]/sizeof(Elf32_Sym);
            newSymbol.st_value = 0;
        }
        if (type == STB_GLOBAL) {
            int* temp = realloc(elfHelper.globalSymbols,(elfHelper.numGlobal+1)*sizeof(int));
            elfHelper.globalSymbols=temp;
            elfHelper.globalSymbols[elfHelper.numGlobal]=elfHelper.sizes[SYM_INDEX]/sizeof(Elf32_Sym);
            elfHelper.numGlobal+=1;
        }

        Elf32_Sym* temp = realloc(elfFile.sections[SYM_INDEX], elfHelper.sizes[SYM_INDEX] + sizeof(Elf32_Sym));
        if (temp == NULL) {
            perror("Failed to resize symtab");
            return;
        }
        elfFile.sections[SYM_INDEX] = temp;
        memcpy(elfFile.sections[SYM_INDEX] + elfHelper.sizes[SYM_INDEX], &newSymbol, sizeof(Elf32_Sym));
        elfHelper.sizes[SYM_INDEX] += sizeof(Elf32_Sym);
    }
}

void startSection(char* sectionName){
    int sectionIndex = getSectionByName(sectionName);

    if (sectionIndex != -1) {//if the section with this name already exists
        elfHelper.activeSection = sectionIndex; // get its index
    } else {
        //if this sections' name doesn't exist in the shstrname(as a part of another sections' name)
        if (getOffset(sectionName, elfFile.sections[SHSTR_INDEX],elfHelper.sizes[SHSTR_INDEX])==0){
            //we have to add the sectionName to the elfFile.sections[SHSTR_INDEX]
            addStringToShstrTab(sectionName);
        }
        Elf32_Shdr newSection = {
            .sh_name = getOffset(sectionName, elfFile.sections[SHSTR_INDEX],elfHelper.sizes[SHSTR_INDEX]),
            .sh_type = SHT_PROGBITS,
            .sh_flags = SHF_ALLOC | SHF_EXECINSTR,
            .sh_addr = 0x00000000u,
            .sh_offset = 0,
            .sh_size = 0,
            .sh_link = 0,
            .sh_info = 0,
            .sh_addralign = 4,
            .sh_entsize = 0
        };

        Elf32_Shdr* temp = realloc(elfFile.sectionHeaderTable, (elfHelper.numOfSections + 1) * sizeof(Elf32_Shdr));
        if (temp == NULL) {
            perror("Failed to resize section header table");
            exit(1);
        }
        elfFile.sectionHeaderTable = temp;
        elfFile.sectionHeaderTable[elfHelper.numOfSections] = newSection;

        uint8_t** tempSections = realloc(elfFile.sections,(elfHelper.numOfSections+1)*sizeof(uint8_t*));
        if (tempSections == NULL) {
            perror("Failed to allocate memory for sections content");
            exit(1);
        }
        elfFile.sections = tempSections;
        elfFile.sections[elfHelper.numOfSections] = NULL;

        elfHelper.sizes = realloc(elfHelper.sizes, (elfHelper.numOfSections+1) * sizeof(size_t));
        elfHelper.sizes[elfHelper.numOfSections] = 0;
        
        elfHelper.activeSection = elfHelper.numOfSections;
        elfHelper.numOfSections++;

        elfHelper.literalSymbolTables = realloc(elfHelper.literalSymbolTables, elfHelper.numOfSections * sizeof(SectionLiteralSymbolTable));
        elfHelper.literalSymbolTables[elfHelper.numOfSections - 1].entries = NULL;
        elfHelper.literalSymbolTables[elfHelper.numOfSections - 1].count = 0;

        elfHelper.wordEntries = realloc(elfHelper.wordEntries, elfHelper.numOfSections * sizeof(WordEntries));
        elfHelper.wordEntries[elfHelper.numOfSections - 1].entries = NULL;
        elfHelper.wordEntries[elfHelper.numOfSections - 1].count = 0;

        elfHelper.relIndex = realloc(elfHelper.relIndex, elfHelper.numOfSections * sizeof(int));
        elfHelper.relIndex[elfHelper.numOfSections-1] = -1;

        addSymbolToSymtab(sectionName,100);
    }
}

void addLabel(char* label) {
    int symbolIndex = -1;
    
    for (int i = 0; i < elfHelper.sizes[SYM_INDEX] / sizeof(Elf32_Sym); i++) {
        Elf32_Sym* sym = (Elf32_Sym*)(elfFile.sections[SYM_INDEX] + i * sizeof(Elf32_Sym));
        if (strcmp(label, elfFile.sections[STR_INDEX] + sym->st_name) == 0) {
            symbolIndex = i;
            break;
        }
    }
    addSymbolToSymtab(label, STB_LOCAL);
}

void AddToWord(char* symbol, size_t addr){
    
    WordEntries* currentTable = &elfHelper.wordEntries[elfHelper.activeSection];

    if (currentTable->count == 0) {
        currentTable->entries = malloc(sizeof(WordEntries));
    } else {
        currentTable->entries = realloc(currentTable->entries, (currentTable->count + 1) * sizeof(WordEntries));
    }
    if (currentTable->entries == NULL) {
        perror("Failed to allocate memory for symbotable entriesl ");
        exit(1);
    }
    WordEntry newEntry;
    newEntry.symbol = strdup(symbol);
    newEntry.offset = addr;
    currentTable->entries[currentTable->count++] = newEntry;
}

void addWord(char* argument,int isLiteral) {
    uint32_t value = 0;

    if (isLiteral) {
        value = strtoul(argument, NULL, 0);

    } else {
        AddToWord(argument,elfHelper.sizes[elfHelper.activeSection]);
    }

    uint8_t* temp = realloc(elfFile.sections[elfHelper.activeSection], elfHelper.sizes[elfHelper.activeSection] + 4);
    if (temp == NULL) {
        perror("Failed to allocate memory for section content");
        exit(1);
    }
    elfFile.sections[elfHelper.activeSection] = temp;
    memcpy(elfFile.sections[elfHelper.activeSection] + elfHelper.sizes[elfHelper.activeSection], &value, 4);
    elfHelper.sizes[elfHelper.activeSection] += 4;
}

void skipBytes(char* argument){
    uint32_t value = strtoul(argument, NULL, 0);
    uint8_t* temp = realloc(elfFile.sections[elfHelper.activeSection], elfHelper.sizes[elfHelper.activeSection] + value);
    if (temp == NULL) {
        perror("Failed to allocate memory for section content");
        exit(1);
    }
    elfFile.sections[elfHelper.activeSection] = temp;
    memset(elfFile.sections[elfHelper.activeSection] + elfHelper.sizes[elfHelper.activeSection], 0, value);
    elfHelper.sizes[elfHelper.activeSection] += value;
}

void writeToSection(uint32_t instruction) {
    uint8_t* temp = realloc(elfFile.sections[elfHelper.activeSection], elfHelper.sizes[elfHelper.activeSection] + sizeof(uint32_t));
    if (temp == NULL) {
        perror("Failed to allocate memory for section content");
        exit(1);
    }
    elfFile.sections[elfHelper.activeSection] = temp;
    memcpy(elfFile.sections[elfHelper.activeSection] + elfHelper.sizes[elfHelper.activeSection], &instruction, sizeof(uint32_t));
    elfHelper.sizes[elfHelper.activeSection] += sizeof(uint32_t);
}

void writeToSpecificSection(uint32_t instruction, int sectionIndex){
    uint8_t* temp = realloc(elfFile.sections[sectionIndex], elfHelper.sizes[sectionIndex] + sizeof(uint32_t));
    if (temp == NULL) {
        perror("Failed to allocate memory for section content");
        exit(1);
    }
    elfFile.sections[sectionIndex] = temp;
    memcpy(elfFile.sections[sectionIndex] + elfHelper.sizes[sectionIndex], &instruction, sizeof(uint32_t));
    elfHelper.sizes[sectionIndex] += sizeof(uint32_t);
}

uint32_t createInstruction(uint8_t i, uint8_t m, uint8_t a, uint8_t b, uint8_t c, int16_t d) {
    uint32_t instruction = 0;

    instruction |= (i & 0xF) << 28;
    instruction |= (m & 0xF) << 24;
    instruction |= (a & 0xF) << 20;
    instruction |= (b & 0xF) << 16;
    instruction |= (c & 0xF) << 12;
    instruction |= (d & 0xFFF);

    return instruction;
}

void reorganizeSymTab() {
    int numSymbols = elfHelper.sizes[SYM_INDEX] / sizeof(Elf32_Sym);
    Elf32_Sym* symtab = (Elf32_Sym*)elfFile.sections[SYM_INDEX];

    Elf32_Sym* localSymbols = malloc(numSymbols * sizeof(Elf32_Sym));
    Elf32_Sym* globalSymbols = malloc(numSymbols * sizeof(Elf32_Sym));
    int* indexMap = malloc(numSymbols * sizeof(int));

    
    int localTotal = 0;
    int globalTotal = 0;
    for (int i = 0; i < numSymbols; i++) {
        if (ELF32_ST_BIND(symtab[i].st_info) == STB_LOCAL) {
            localTotal+=1;
        } else {
            globalTotal+=1;
        }
    }
    int localCount = 0;
    int globalCount = 0;
    for (int i = 0; i < numSymbols; i++) {
        if (ELF32_ST_BIND(symtab[i].st_info) == STB_LOCAL) {
            localSymbols[localCount] = symtab[i];
            indexMap[i] = localCount++;
        } else {
            globalSymbols[globalCount] = symtab[i];
            indexMap[i] = localTotal + globalCount++;
        }
    }

    memcpy(symtab, localSymbols, localCount * sizeof(Elf32_Sym));
    memcpy(symtab + localCount, globalSymbols, globalCount * sizeof(Elf32_Sym));

    elfFile.sectionHeaderTable[SYM_INDEX].sh_info = localCount;

    for (int i = 0; i < numSymbols; i++) {
        if (symtab[i].st_shndx < SHN_LORESERVE) {
            symtab[i].st_shndx = indexMap[symtab[i].st_shndx];
        }
    }

    for (int i = 0; i < elfHelper.numOfSections; i++) {
        if (elfFile.sectionHeaderTable[i].sh_type == SHT_RELA) {
            Elf32_Rela* relas = (Elf32_Rela*)elfFile.sections[i];
            int numRelas = elfHelper.sizes[i] / sizeof(Elf32_Rela);

            for (int j = 0; j < numRelas; j++) {
                int oldSymIndex = ELF32_R_SYM(relas[j].r_info);
                int newSymIndex = indexMap[oldSymIndex];
                relas[j].r_info = ELF32_R_INFO(newSymIndex, ELF32_R_TYPE(relas[j].r_info));
            }
        }
    }
    free(localSymbols);
    free(globalSymbols);
    free(indexMap);
}

void finalizeSections(){
    int numberOfSections = elfHelper.numOfSections;
    for (size_t i = 4; i < numberOfSections; i++)
    {
        finishSection(i);
    }
    reorganizeSymTab();
}

void writeElfFile(const char* filename) {
    finalizeSections();
    elfFile.elfHeader.e_shnum = elfHelper.numOfSections;
    FILE* outputFile = fopen(filename, "wb");
    if (!outputFile) {
        perror("Failed to open output file");
        exit(1);
    }
    fwrite(&elfFile.elfHeader, sizeof(Elf32_Ehdr), 1, outputFile);
    uint32_t currentOffset = sizeof(Elf32_Ehdr);
    for (int i = 0; i < elfHelper.numOfSections; i++) {
        uint32_t align = elfFile.sectionHeaderTable[i].sh_addralign;
        if (align > 1) {
            uint32_t alignedOffset = (currentOffset + align - 1) & ~(align - 1);
            if (alignedOffset > currentOffset) {
                uint32_t paddingSize = alignedOffset - currentOffset;
                uint8_t padding[paddingSize];
                memset(padding, 0, paddingSize);
                fwrite(padding, paddingSize, 1, outputFile);
                currentOffset = alignedOffset;
            }
        }
        elfFile.sectionHeaderTable[i].sh_offset = currentOffset;
        elfFile.sectionHeaderTable[i].sh_size = elfHelper.sizes[i];
        fwrite(elfFile.sections[i], elfHelper.sizes[i], 1, outputFile);
        currentOffset += elfHelper.sizes[i];
    }
    fwrite(elfFile.sectionHeaderTable, sizeof(Elf32_Shdr), elfHelper.numOfSections, outputFile);

    elfFile.elfHeader.e_shoff = currentOffset;
    fseek(outputFile, 32, 0);//update the e_shoff field in the elfHeader
    fwrite(&elfFile.elfHeader.e_shoff, sizeof(elfFile.elfHeader.e_shoff), 1, outputFile);
    fclose(outputFile);
}


void addLiteral(char* literal,size_t addr){
    uint32_t literalValue = strtoul(literal, NULL, 0);
    SectionLiteralSymbolTable* currentTable = &elfHelper.literalSymbolTables[elfHelper.activeSection];
    for (size_t i = 0; i < currentTable->count; i++) {
        if (currentTable->entries[i].isLiteral && currentTable->entries[i].value == literalValue) {
            currentTable->entries[i].toPatch = realloc(currentTable->entries[i].toPatch, (currentTable->entries[i].patchNum + 1) * sizeof(size_t));
            if (currentTable->entries[i].toPatch == NULL) {
                perror("Failed to allocate memory for toPatch array");
                exit(1);
            }
            currentTable->entries[i].toPatch[currentTable->entries[i].patchNum++] = addr;
            return;
        }
    }
    if (currentTable->count == 0) {
        currentTable->entries = malloc(sizeof(LiteralSymbolEntry));
    } else {
        currentTable->entries = realloc(currentTable->entries, (currentTable->count + 1) * sizeof(LiteralSymbolEntry));
    }
    if (currentTable->entries == NULL) {
        perror("Failed to allocate memory for literal table entries");
        exit(1);
    }

    LiteralSymbolEntry newEntry;
    newEntry.value = literalValue;
    newEntry.isLiteral = 1;
    newEntry.entrySize = sizeof(literalValue);
    newEntry.offset = 0;
    newEntry.patchNum = 1;
    newEntry.toPatch = malloc(sizeof(size_t));
    if (newEntry.toPatch == NULL) {
        perror("Failed to allocate memory for patch list");
        exit(1);
    }
    newEntry.toPatch[0] = addr;
    currentTable->entries[currentTable->count++] = newEntry;
}

void addSymbol(char* symbol, size_t addr) {
    SectionLiteralSymbolTable* currentTable = &elfHelper.literalSymbolTables[elfHelper.activeSection];
    for (size_t i = 0; i < currentTable->count; i++) {
        if (!currentTable->entries[i].isLiteral && strcmp(currentTable->entries[i].symbol, symbol) == 0) {
            currentTable->entries[i].toPatch = realloc(currentTable->entries[i].toPatch, (currentTable->entries[i].patchNum + 1) * sizeof(size_t));
            if (currentTable->entries[i].toPatch == NULL) {
                perror("Failed to allocate memory for toPatch array");
                exit(1);
            }
            currentTable->entries[i].toPatch[currentTable->entries[i].patchNum++] = addr;
            return;
        }
    }

    if (currentTable->count == 0) {
        currentTable->entries = malloc(sizeof(LiteralSymbolEntry));
    } else {
        currentTable->entries = realloc(currentTable->entries, (currentTable->count + 1) * sizeof(LiteralSymbolEntry));
    }
    if (currentTable->entries == NULL) {
        perror("Failed to allocate memory for symbol table entries");
        exit(1);
    }
    LiteralSymbolEntry newEntry;
    newEntry.symbol = strdup(symbol);
    newEntry.isLiteral = 0;
    newEntry.value = 0;
    newEntry.entrySize = 32;
    newEntry.offset = 0;
    newEntry.patchNum = 1;
    newEntry.toPatch = malloc(sizeof(size_t));
    if (newEntry.toPatch == NULL) {
        perror("Failed to allocate memory for patch list");
        exit(1);
    }
    newEntry.toPatch[0] = addr;
    currentTable->entries[currentTable->count++] = newEntry;
}


void finishSection(int sectionIndex){
    SectionLiteralSymbolTable currentTable = elfHelper.literalSymbolTables[sectionIndex];
    uint32_t jmpAddr;
    if(currentTable.count>0){//we need to jump over the stringliteral pool
        jmpAddr = elfHelper.sizes[sectionIndex];
        uint32_t instr;
        instr = createInstruction(0b0011,0b0000,0b1111,0b0000,0b0000,0b0000);
        writeToSpecificSection(instr,sectionIndex);
    }
    for (size_t i = 0; i < currentTable.count; i++)
    {
        LiteralSymbolEntry entry = currentTable.entries[i];
        size_t entrySize = entry.entrySize;
        if(entry.isLiteral){
            for (size_t i = 0; i < entry.patchNum; i++)
            {
                size_t addr = entry.toPatch[i];
                int32_t offset = elfHelper.sizes[sectionIndex] - addr;

                if (offset < -2048 || offset > 2047) {
                    fprintf(stderr, "Offset out of 12-bit range\n");
                    exit(EXIT_FAILURE);
                }
                uint16_t originalValue;
                memcpy(&originalValue, elfFile.sections[sectionIndex] + addr, sizeof(uint16_t));
                uint16_t newValue = (originalValue & 0xF000) | ((uint16_t)(offset & 0x0FFF)) -4;
                memcpy(elfFile.sections[sectionIndex] + addr, &newValue, sizeof(uint16_t));
            }
        
            uint8_t* temp = realloc(elfFile.sections[sectionIndex], elfHelper.sizes[sectionIndex] + entrySize);
            if (temp == NULL) {
                perror("Failed to allocate memory for section content");
                exit(1);
            }
            entry.offset = elfHelper.sizes[sectionIndex];//offset ovog literala u okviru trenutne sekcije
            elfFile.sections[sectionIndex] = temp;
            memcpy(elfFile.sections[sectionIndex] + elfHelper.sizes[sectionIndex], &entry.value, entrySize);
            elfHelper.sizes[sectionIndex] += entrySize;
        }else{
            for (size_t k = 0; k < entry.patchNum; k++) {
                size_t addr = entry.toPatch[k];
                int32_t offset = elfHelper.sizes[sectionIndex] - addr;

                if (offset < -2048 || offset > 2047) {
                    fprintf(stderr, "Offset out of 12-bit range\n");
                    exit(EXIT_FAILURE);
                }

                uint16_t originalValue;
                memcpy(&originalValue, elfFile.sections[sectionIndex] + addr, sizeof(uint16_t));
                uint16_t newValue = (originalValue & 0xF000) | ((uint16_t)(offset & 0x0FFF)) -4;
                memcpy(elfFile.sections[sectionIndex] + addr, &newValue, sizeof(uint16_t));
            }

            entry.offset = elfHelper.sizes[sectionIndex];//offset ovog simbola u okviru trenutne sekcije

            uint8_t* temp;
            if (elfHelper.relIndex[sectionIndex] == -1) {
                char relSectionName[50];
                snprintf(relSectionName, sizeof(relSectionName), ".rela%s", (char*)(elfFile.sections[SHSTR_INDEX] + elfFile.sectionHeaderTable[sectionIndex].sh_name));

                addStringToShstrTab(relSectionName);

                Elf32_Shdr newRelSectionHeader = {
                    .sh_name = getOffset(relSectionName, elfFile.sections[SHSTR_INDEX], elfHelper.sizes[SHSTR_INDEX]),
                    .sh_type = SHT_RELA,
                    .sh_flags = 0,
                    .sh_addr = 0x00000000u,
                    .sh_offset = 0,
                    .sh_size = 0,
                    .sh_link = SYM_INDEX,
                    .sh_info = sectionIndex,
                    .sh_addralign = 4,
                    .sh_entsize = sizeof(Elf32_Rela),
                };

                Elf32_Shdr* temp = realloc(elfFile.sectionHeaderTable, (elfHelper.numOfSections + 1) * sizeof(Elf32_Shdr));
                if (temp == NULL) {
                    perror("Failed to resize section header table for rel section");
                    exit(1);
                }
                elfFile.sectionHeaderTable = temp;
                elfFile.sectionHeaderTable[elfHelper.numOfSections] = newRelSectionHeader;

                uint8_t** tempSections = realloc(elfFile.sections, (elfHelper.numOfSections + 1) * sizeof(uint8_t*));
                if (tempSections == NULL) {
                    perror("Failed to allocate memory for new rel section content");
                    exit(1);
                }
                elfFile.sections = tempSections;
                elfFile.sections[elfHelper.numOfSections] = NULL;

                size_t* tempSizes = realloc(elfHelper.sizes, (elfHelper.numOfSections + 1) * sizeof(size_t));
                if (tempSizes == NULL) {
                    perror("Failed to allocate memory for section sizes array");
                    exit(1);
                }
                elfHelper.sizes = tempSizes;
                elfHelper.sizes[elfHelper.numOfSections] = 0;

                elfHelper.relIndex[sectionIndex] = elfHelper.numOfSections;

                elfHelper.numOfSections++;
            }

            uint32_t r_offset = elfHelper.sizes[sectionIndex]; 

            uint32_t r_info;
            int32_t r_addend = 0;
            int symIndex = getSymbolIndex(entry.symbol);
            if (symIndex == -1) {
                printf("Symbol %s not found in symtab\n",entry.symbol);
                exit(-1);
            }
            Elf32_Sym* symbol = getSymbolByIndex(symIndex);
            if (isGlobalSymbol(symIndex)) {
                r_info = ELF32_R_INFO(symIndex, R_X86_64_32);
                r_addend = 0;
            } else {
                r_info = ELF32_R_INFO(symbol->st_shndx, R_X86_64_32);
                r_addend = symbol->st_value;
            }

            Elf32_Rela relocationEntry = {
                .r_offset = r_offset,
                .r_info = r_info,
                .r_addend =  r_addend
            };

            size_t relSectionIndex = elfHelper.relIndex[sectionIndex];
            elfFile.sections[relSectionIndex] = realloc(elfFile.sections[relSectionIndex], elfHelper.sizes[relSectionIndex] + sizeof(Elf32_Rela));
            if (elfFile.sections[relSectionIndex] == NULL) {
                perror("Failed to allocate memory for relocation entry");
                exit(1);
            }
            memcpy(elfFile.sections[relSectionIndex] + elfHelper.sizes[relSectionIndex], &relocationEntry, sizeof(Elf32_Rela));
            elfHelper.sizes[relSectionIndex] += sizeof(Elf32_Rela);

            temp = realloc(elfFile.sections[sectionIndex], elfHelper.sizes[sectionIndex] + 4);
            if (temp == NULL) {
                perror("Failed to allocate memory for section content");
                exit(1);
            }
            elfFile.sections[sectionIndex] = temp;
            memset(elfFile.sections[sectionIndex] + elfHelper.sizes[sectionIndex], 0, 4);

            elfHelper.sizes[sectionIndex] += 4;
        }
    }
    if(currentTable.count>0){
        uint32_t originalValue;
        memcpy(&originalValue, elfFile.sections[sectionIndex] + jmpAddr, sizeof(uint32_t));
        uint32_t offset = elfHelper.sizes[sectionIndex] - jmpAddr -4;//PC is 4 bytes ahead in the runtime
        uint32_t newValue = (originalValue & 0xFFFFF000) | ((uint32_t)(offset & 0x0FFF));
        memcpy(elfFile.sections[sectionIndex] + jmpAddr, &newValue, sizeof(uint32_t));
    }
    /*WORD ENTRIES*/
    WordEntries wordentries = elfHelper.wordEntries[sectionIndex];
    for (size_t i = 0; i < wordentries.count; i++)
    {
        uint8_t* temp;
        if (elfHelper.relIndex[sectionIndex] == -1) {
            char relSectionName[50];
            snprintf(relSectionName, sizeof(relSectionName), ".rela%s", (char*)(elfFile.sections[SHSTR_INDEX] + elfFile.sectionHeaderTable[sectionIndex].sh_name));

            addStringToShstrTab(relSectionName);

            Elf32_Shdr newRelSectionHeader = {
                .sh_name = getOffset(relSectionName, elfFile.sections[SHSTR_INDEX], elfHelper.sizes[SHSTR_INDEX]),
                .sh_type = SHT_RELA,
                .sh_flags = 0,
                .sh_addr = 0x00000000u,
                .sh_offset = 0,
                .sh_size = 0,
                .sh_link = SYM_INDEX,
                .sh_info = sectionIndex,
                .sh_addralign = 4,
                .sh_entsize = sizeof(Elf32_Rela),
            };

            Elf32_Shdr* temp = realloc(elfFile.sectionHeaderTable, (elfHelper.numOfSections + 1) * sizeof(Elf32_Shdr));
            if (temp == NULL) {
                perror("Failed to resize section header table for rel section");
                exit(1);
            }
            elfFile.sectionHeaderTable = temp;
            elfFile.sectionHeaderTable[elfHelper.numOfSections] = newRelSectionHeader;

            uint8_t** tempSections = realloc(elfFile.sections, (elfHelper.numOfSections + 1) * sizeof(uint8_t*));
            if (tempSections == NULL) {
                perror("Failed to allocate memory for new rel section content");
                exit(1);
            }
            elfFile.sections = tempSections;
            elfFile.sections[elfHelper.numOfSections] = NULL;

            size_t* tempSizes = realloc(elfHelper.sizes, (elfHelper.numOfSections + 1) * sizeof(size_t));
            if (tempSizes == NULL) {
                perror("Failed to allocate memory for section sizes array");
                exit(1);
            }
            elfHelper.sizes = tempSizes;
            elfHelper.sizes[elfHelper.numOfSections] = 0;

            elfHelper.relIndex[sectionIndex] = elfHelper.numOfSections;

            elfHelper.numOfSections++;
        }

        uint32_t r_offset = wordentries.entries[i].offset;

        uint32_t r_info;
        int32_t r_addend = 0;
        int symIndex = getSymbolIndex(wordentries.entries[i].symbol);
        if (symIndex == -1) {
            printf("Symbol %s not found in symtab\n",wordentries.entries[i].symbol);
            exit(-1);
        }
        Elf32_Sym* symbol = getSymbolByIndex(symIndex);
        if (isGlobalSymbol(symIndex)) {
            r_info = ELF32_R_INFO(symIndex, R_X86_64_32);
            r_addend = 0;
        } else {
            r_info = ELF32_R_INFO(symbol->st_shndx, R_X86_64_32);
            r_addend = symbol->st_value;
        }

        Elf32_Rela relocationEntry = {
            .r_offset = r_offset,
            .r_info = r_info,
            .r_addend =  r_addend
        };

        size_t relSectionIndex = elfHelper.relIndex[sectionIndex];
        elfFile.sections[relSectionIndex] = realloc(elfFile.sections[relSectionIndex], elfHelper.sizes[relSectionIndex] + sizeof(Elf32_Rela));
        if (elfFile.sections[relSectionIndex] == NULL) {
            perror("Failed to allocate memory for relocation entry");
            exit(1);
        }
        memcpy(elfFile.sections[relSectionIndex] + elfHelper.sizes[relSectionIndex], &relocationEntry, sizeof(Elf32_Rela));
        elfHelper.sizes[relSectionIndex] += sizeof(Elf32_Rela);
    }
}

void print_uint32_as_bytes(uint32_t value) {
    unsigned char bytes[4];
    
    bytes[0] = (value >> 24) & 0xFF;
    bytes[1] = (value >> 16) & 0xFF;
    bytes[2] = (value >> 8) & 0xFF;
    bytes[3] = value & 0xFF;
    
    for (int i = 0; i < 4; i++) {
        printf("%02x ", bytes[i]);
    }
    printf("\n");
}

int main(int argc, char **argv) {
    char* output_file = "output.o";
    if (argc > 1) {
        input_file = fopen(argv[argc-1], "r");
        if (!input_file) {
            perror("fopen");
            return -1;
        }
        if (argc>2 && strcmp(argv[1],"-o")==0){
            output_file = argv[2]; 
        }else{
            perror("Bad parameters");
            return -1;
        }
    }else{
        printf("Taking input from the console...\n");
        input_file = stdin;
    }
    char *line;

    initializeStructures();

    while ((line = readNextLine()) != NULL) {
        yy_scan_string(line);
        yyparse();
        if(parsed_line.empty == 1){freeUsedMemory();continue;};
        char* mnemonic = parsed_line.mnemonic;
        char* label = parsed_line.label;
        int reg;
        int gpr;
        int gpr1;
        int gpr2;
        int regS;
        int regD;
        char* symbol;
        char* literal;
        uint32_t instr = 10000000;
        if(label!=NULL){
            addLabel(label);
        }
        if(mnemonic == NULL){freeUsedMemory();continue;}
        if(strcmp(mnemonic,".global")==0){

            for (int i = 0; i < parsed_line.arg_count; i++) {
                addSymbolToSymtab(parsed_line.arguments[i],STB_GLOBAL);
            }

        }else if (strcmp(mnemonic,".extern")==0 ){

            for (int i = 0; i < parsed_line.arg_count; i++) {
                addSymbolToSymtab(parsed_line.arguments[i],STB_GLOBAL);
            }
        }
        else if(strcmp(mnemonic,".section")==0){

            startSection(parsed_line.arguments[0]);

        }else if(strcmp(mnemonic,".word")==0){
            for (int i = 0; i < parsed_line.arg_count; i++) {
                addWord(parsed_line.arguments[i],parsed_line.isLiteral[i]);
            }
        }else if(strcmp(mnemonic,".skip")==0){
            skipBytes(parsed_line.arguments[0]);
        }else if (strcmp(mnemonic,".end")==0){
            break;
        }else if(strcmp(mnemonic,"halt")==0){
            instr = createInstruction(0,0,0,0,0,0);

        }else if(strcmp(mnemonic,"int")==0){
            instr = createInstruction(1,0,0,0,0,0);

        }else if(strcmp(mnemonic,"iret")==0){
            instr = createInstruction(0b1001,0b0111,0b0000,0b1110,0b1110,4);//pop status(0b0000)
            writeToSection(instr);
            instr = createInstruction(0b1001,0b0011,0b1111,0b1110,0b1110,4);//pop pc(0b1111)

        }else if(strcmp(mnemonic,"call")==0){
            if (parsed_line.info==0){
                char* literal = parsed_line.arguments[0];
                addLiteral(literal,elfHelper.sizes[elfHelper.activeSection]);
            }else{
                char* symbol = parsed_line.arguments[0];
                addSymbol(symbol,elfHelper.sizes[elfHelper.activeSection]);
            }
            instr = createInstruction(0b0010,0b0001,0b1111,0b0000,0b0000,0b0000);
        }else if(strcmp(mnemonic,"ret")==0){
            instr = createInstruction(0b1001,0b0011,0b1111,0b1110,0b1110,4);//pop pc(0b1111)
            // pc = r15
            // sp = r14
        }else if(strcmp(mnemonic,"jmp")==0){
            if (parsed_line.info==0){
                char* literal = parsed_line.arguments[0];
                addLiteral(literal,elfHelper.sizes[elfHelper.activeSection]);
                instr = createInstruction(0b0011,0b1000,0b1111,0b0000,0b0000,0b0000);
            }else{
                char* symbol = parsed_line.arguments[0];
                addSymbol(symbol,elfHelper.sizes[elfHelper.activeSection]);
                instr = createInstruction(0b0011,0b1000,0b1111,0b0000,0b0000,0b0000);
            }

        }else if(strcmp(mnemonic,"beq")==0){
            int gpr1 = atoi(parsed_line.arguments[0]);
            int gpr2 = atoi(parsed_line.arguments[1]);
            if (parsed_line.info==0){
                char* literal = parsed_line.arguments[2];
                addLiteral(literal,elfHelper.sizes[elfHelper.activeSection]);
            }else{
                char* symbol = parsed_line.arguments[2];
                addSymbol(symbol,elfHelper.sizes[elfHelper.activeSection]);
            }
            instr = createInstruction(0b0011,0b1001,0b1111,gpr1,gpr2,0);
        }else if(strcmp(mnemonic,"bne")==0){
            int gpr1 = atoi(parsed_line.arguments[0]);
            int gpr2 = atoi(parsed_line.arguments[1]);
            if (parsed_line.info==0){
                char* literal = parsed_line.arguments[2];
                addLiteral(literal,elfHelper.sizes[elfHelper.activeSection]);
            }else{
                char* symbol = parsed_line.arguments[2];
                addSymbol(symbol,elfHelper.sizes[elfHelper.activeSection]);
            }
            instr = createInstruction(0b0011,0b1010,0b1111,gpr1,gpr2,0);
        }else if(strcmp(mnemonic,"bgt")==0){
            int gpr1 = atoi(parsed_line.arguments[0]);
            int gpr2 = atoi(parsed_line.arguments[1]);
            if (parsed_line.info==0){
                char* literal = parsed_line.arguments[2];
                addLiteral(literal,elfHelper.sizes[elfHelper.activeSection]);
            }else{
                char* symbol = parsed_line.arguments[2];
                addSymbol(symbol,elfHelper.sizes[elfHelper.activeSection]);
            }
            instr = createInstruction(0b0011,0b1011,0b1111,gpr1,gpr2,0);
        }else if(strcmp(mnemonic,"push")==0){
            reg= atoi(parsed_line.arguments[0]);
            instr = createInstruction(0b1000,0b0001,0b1110,0b1110,reg,-4);
            
        }else if(strcmp(mnemonic,"pop")==0){
            reg= atoi(parsed_line.arguments[0]);
            instr = createInstruction(0b1001,0b0011,reg,0b1110,0b1110,4);
            
        }else if(strcmp(mnemonic,"xchg")==0){
            regS= atoi(parsed_line.arguments[0]);
            regD= atoi(parsed_line.arguments[1]);
            instr = createInstruction(0b0100,0b0000,0b0000,regS,regD,0);
            
        }else if(strcmp(mnemonic,"add")==0){
            regS= atoi(parsed_line.arguments[0]);
            regD= atoi(parsed_line.arguments[1]);
            instr = createInstruction(0b0101,0b0000,regD,regD,regS,0);
            
        }else if(strcmp(mnemonic,"sub")==0){
            regS= atoi(parsed_line.arguments[0]);
            regD= atoi(parsed_line.arguments[1]);
            instr = createInstruction(0b0101,0b0001,regD,regD,regS,0);
            
        }else if(strcmp(mnemonic,"mul")==0){
            regS= atoi(parsed_line.arguments[0]);
            regD= atoi(parsed_line.arguments[1]);
            instr = createInstruction(0b0101,0b0010,regD,regD,regS,0);
            
        }else if(strcmp(mnemonic,"div")==0){
            regS= atoi(parsed_line.arguments[0]);
            regD= atoi(parsed_line.arguments[1]);
            instr = createInstruction(0b0101,0b0011,regD,regD,regS,0);
            
        }else if(strcmp(mnemonic,"not")==0){
            reg= atoi(parsed_line.arguments[0]);
            instr = createInstruction(0b0110,0b0000,reg,reg,0b0000,0);
            
        }else if(strcmp(mnemonic,"and")==0){
            regS= atoi(parsed_line.arguments[0]);
            regD= atoi(parsed_line.arguments[1]);
            instr = createInstruction(0b0110,0b0001,regD,regD,regS,0);
            
        }else if(strcmp(mnemonic,"or")==0){
            regS= atoi(parsed_line.arguments[0]);
            regD= atoi(parsed_line.arguments[1]);
            instr = createInstruction(0b0110,0b0010,regD,regD,regS,0);
            
        }else if(strcmp(mnemonic,"xor")==0){
            regS= atoi(parsed_line.arguments[0]);
            regD= atoi(parsed_line.arguments[1]);
            instr = createInstruction(0b0110,0b0011,regD,regD,regS,0);
            
        }else if(strcmp(mnemonic,"shl")==0){
            regS= atoi(parsed_line.arguments[0]);
            regD= atoi(parsed_line.arguments[1]);
            instr = createInstruction(0b0111,0b0000,regD,regD,regS,0);
            
        }else if(strcmp(mnemonic,"shr")==0){
            regS= atoi(parsed_line.arguments[0]);
            regD= atoi(parsed_line.arguments[1]);
            instr = createInstruction(0b0111,0b0001,regD,regD,regS,0);
            
        }else if(strcmp(mnemonic,"ld")==0){
            // gpr <= operand
            switch (parsed_line.info) {
                case 0:
                    literal= parsed_line.arguments[0];
                    gpr= atoi(parsed_line.arguments[1]);
                    addLiteral(literal,elfHelper.sizes[elfHelper.activeSection]);
                    instr = createInstruction(0b1001,0b0010,gpr,0b1111,0,0);
                    break;
                case 1:
                    symbol= parsed_line.arguments[0];
                    gpr= atoi(parsed_line.arguments[1]);
                    addSymbol(symbol,elfHelper.sizes[elfHelper.activeSection]);
                    instr = createInstruction(0b1001,0b0010,gpr,0b1111,0,0);
                    break;
                case 2:
                    literal= parsed_line.arguments[0];
                    gpr= atoi(parsed_line.arguments[1]);
                    addLiteral(literal,elfHelper.sizes[elfHelper.activeSection]);
                    instr = createInstruction(0b1001,0b0010,gpr,0b1111,0,0);
                    writeToSection(instr);
                    instr = createInstruction(0b1001,0b0010,gpr,0,0,0);
                    break;
                case 3:
                    symbol= parsed_line.arguments[0];
                    gpr= atoi(parsed_line.arguments[1]);
                    addSymbol(symbol,elfHelper.sizes[elfHelper.activeSection]);
                    instr = createInstruction(0b1001,0b0010,gpr,0b1111,0,0);
                    writeToSection(instr);
                    instr = createInstruction(0b1001,0b0010,gpr,gpr,0,0);
                    break;
                case 4:
                    reg= atoi(parsed_line.arguments[0]);
                    gpr= atoi(parsed_line.arguments[1]);
                    instr = createInstruction(0b1001,0b0001,gpr,reg,0b0000,0);
                    break;
                case 5:
                    reg= atoi(parsed_line.arguments[0]);
                    gpr= atoi(parsed_line.arguments[1]);
                    instr = createInstruction(0b1001,0b0010,gpr,reg,0b0000,0);
                    break;
                case 6:
                    reg= atoi(parsed_line.arguments[0]);
                    int16_t literal1= strtoul(parsed_line.arguments[1], NULL, 0);
                    gpr= atoi(parsed_line.arguments[2]);
                    if (literal1 < -2048 || literal1 > 2047) {
                        fprintf(stderr, "Error: literal %d is larger than 12 bits\n", literal1);
                        exit(-1);
                    }
                    instr = createInstruction(0b1001,0b0010,gpr,reg,0b0000,literal1);
                    break;
                case 7://gpr<-MEM[symbol + reg]
                    reg= atoi(parsed_line.arguments[0]);
                    char* simbol = parsed_line.arguments[1];
                    gpr= atoi(parsed_line.arguments[2]);
                    addSymbol(simbol,elfHelper.sizes[elfHelper.activeSection]);
                    instr = createInstruction(0b1001,0b0010,gpr,0b1111,0b0000,0);//gpr<-MEM[PC+DDDD](simbol)
                    writeToSection(instr);
                    instr = createInstruction(0b1001,0b0010,gpr,gpr,reg,0);//gpr<-MEM[gpr(simbol)+reg]
                    break;
                default:
                    perror("An error occured!");
                    return -1;
            }
        }else if(strcmp(mnemonic,"st")==0){//gpr,operand
            //operand <= gpr
            gpr= atoi(parsed_line.arguments[0]);
            switch (parsed_line.info) {
                case 0:
                    perror("Invalid instruction + address combination");
                    exit(-1);
                case 1:
                    perror("Invalid instruction + address combination");
                    exit(-1);
                case 2:
                    literal= parsed_line.arguments[1];
                    addLiteral(literal,elfHelper.sizes[elfHelper.activeSection]);
                    instr = createInstruction(0b1000,0b0010,0b1111,0b0000,gpr,0);
                    break;
                case 3:
                    symbol= parsed_line.arguments[1];
                    addSymbol(symbol,elfHelper.sizes[elfHelper.activeSection]);
                    instr = createInstruction(0b1000,0b0010,0b1111,0b0000,gpr,0);
                    break;
                case 4:
                    reg= atoi(parsed_line.arguments[1]);
                    instr = createInstruction(0b1001,0b0001,reg,gpr,0b0000,0);
                    break;
                case 5:
                    reg= atoi(parsed_line.arguments[1]);
                    instr = createInstruction(0b1000,0b0000,reg,0b0000,gpr,0);
                    break;
                case 6:
                    reg= atoi(parsed_line.arguments[1]);
                    int16_t literal1= strtoul(parsed_line.arguments[2], NULL, 0);
                    if (literal1 < -2048 || literal1 > 2047) {
                        fprintf(stderr, "Error: literal %d is larger than 12 bits\n", literal1);
                        exit(-1);
                    }
                    instr = createInstruction(0b1000,0b0000,reg,0b0000,gpr,literal1);
                    break;

                case 7:
                    reg= atoi(parsed_line.arguments[1]);
                    char* simbol = parsed_line.arguments[2];
                    instr = createInstruction(0b1000,0b0001,0b1110,0b1110,0b0011,-4);//push r3
                    writeToSection(instr);
                    addSymbol(simbol,elfHelper.sizes[elfHelper.activeSection]);
                    instr = createInstruction(0b1001,0b0010,0b0011,0b1111,0b0000,0);//r3<=MEM[PC+offset](simbol)
                    writeToSection(instr);
                    instr = createInstruction(0b1000,0b0000,reg,0b0011,gpr,0);//MEM[reg + r3(simbol)]<-gpr
                    writeToSection(instr);
                    instr = createInstruction(0b1001,0b0011,0b0011,0b1110,0b1110,4);//pop r3
                    break;
                default:
                    perror("An error occured!");
                    return -1;
            }
        }else if(strcmp(mnemonic,"csrrd")==0){
            int csr=0;
            if (strcmp(parsed_line.arguments[0],"status")==0){
                csr = 0;
            }else if (strcmp(parsed_line.arguments[0],"handler")==0){
                csr = 1;
            }else if (strcmp(parsed_line.arguments[0],"cause")==0){
                csr = 2;
            }
            gpr= atoi(parsed_line.arguments[1]);
            instr = createInstruction(0b1001,0b0000,gpr,csr,0b0000,0);
            
        }else if(strcmp(mnemonic,"csrwr")==0){
            gpr= atoi(parsed_line.arguments[0]);
            int csr=0;
            if (strcmp(parsed_line.arguments[1],"status")==0){
                csr = 0;
            }else if (strcmp(parsed_line.arguments[1],"handler")==0){
                csr = 1;
            }else if (strcmp(parsed_line.arguments[1],"cause")==0){
                csr = 2;
            }
            instr = createInstruction(0b1001,0b0100,csr,gpr,0b0000,0);
        }else{

        }
        if(instr!=10000000){writeToSection(instr);}
        freeUsedMemory();
    }
    writeElfFile(output_file);

    fclose(input_file);
    return 0;
}
