#include <vector>
#include <string>
#include <map>
using namespace std;

vector<uint32_t> r;//registers
uint32_t irq;
vector<uint32_t> csr;
map<uint32_t, uint8_t> mem;