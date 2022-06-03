#include "common.h"

bool common_bit_set(long long value, byte position) { return value & (1L << position); }

// I could do this through non-void methods with returns in one copy,
// but this variant is slightly faster, and needs less typing in client code
#define M_common(SUFFIX, TYPE) \
    void common_set_bit##SUFFIX(TYPE *variable, byte position)    { *variable |= 1L << position;    } \
    void common_unset_bit##SUFFIX(TYPE *variable, byte position)  { *variable &= ~(1L << position); } \
    void common_toggle_bit##SUFFIX(TYPE *variable, byte position) { *variable ^= 1L << position;    } \
    void common_modify_bit##SUFFIX(TYPE *variable, byte position, bool set) \
        { set ? common_set_bit##SUFFIX(variable, position) : common_unset_bit##SUFFIX(variable, position); }

M_common(b, byte)
M_common(w, word)
M_common(d, dword)
M_common(q, qword)
#ifdef YATCPU
void* memcpy(void* src, const void * dst, unsigned int size)
{
    int rem = size & 0x3;
    int words = size >> 2;
    int* word_dst = (int*) dst;
    int* word_src = (int*) src;
    for (int i = 0; i < words; ++i) {
        *(word_dst++) = *(word_src++);
    }
    char *char_dst = (char*) word_dst;
    char *char_src = (char*) word_src;
    for (int i = 0; i < rem; ++i) {
        *(char_dst++) = *(char_src++);
    }
    return (void*) dst;
}

int memcmp(const void* va, const void* vb, unsigned int size) {
    char* a = (char *) va;
    char* b = (char *) vb;
    for (int i = 0; i < size; ++i) {
        int result = *a - *b;
        if (result != 0) return result;
        ++a;
        ++b;
    }
    return 0;    
}

unsigned int
__mulsi3 (unsigned int a, unsigned int b)
{
  unsigned int r = 0;

  while (a)
    {
      if (a & 1)
	r += b;
      a >>= 1;
      b <<= 1;
    }
  return r;
}
#endif
