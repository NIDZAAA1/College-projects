#include "elf.h"
#include <vector>
#include <string>
#include <map>
using namespace std;

class Section{
  public:
    uint32_t startAddr;
    int fixed;
    int written;
    vector<uint8_t> content;
    char* name;
    int fileIndex;
    Elf32_Shdr* sectionHeader;
};

vector<Section> sections;
map<char*,uint32_t> fixedPlaces;
map<uint32_t, uint8_t> memory;
map<char*,uint32_t> symbolAddresses;