#ifndef ASSEMBLER_H
#define ASSEMBLER_H
#include <elf.h>

void checkIfAllGlobalSymbolsAreDefined();

typedef struct {
    char *label;
    char *mnemonic;//instruction or assembly directive
    char **arguments;
    int arg_count;
    int empty;
    int info;
    int* isLiteral;
} ParsedLine;


typedef struct LiteralSymbolEntry {
    uint32_t offset;
    char* symbol;
    int isLiteral;
    uint32_t value;
    size_t entrySize;
    size_t* toPatch;
    size_t patchNum;
} LiteralSymbolEntry;

typedef struct SectionLiteralSymbolTable {
    LiteralSymbolEntry* entries;
    size_t count;
} SectionLiteralSymbolTable;

typedef struct WordEntry{
    char* symbol;
    uint32_t offset;
}WordEntry;

typedef struct WordEntries{
    WordEntry* entries;
    size_t count;
}WordEntries;

typedef struct ElfHelper
{
    int16_t activeSection;
    size_t numOfSections;
    size_t* sizes;
    int* globalSymbols;
    int numGlobal;
    SectionLiteralSymbolTable* literalSymbolTables;
    int* relIndex;
    WordEntries* wordEntries;
}ElfHelper;


typedef struct ElfFile
{
    Elf32_Ehdr elfHeader;
    uint8_t** sections;
    Elf32_Shdr* sectionHeaderTable;
}ElfFile;

#endif

