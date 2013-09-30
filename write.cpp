#include <write.h>

#define TYPE_NAME(TYPE, NAME) \
  template<> const char* type_name<TYPE>::value = NAME;

TYPE_NAME(uint8_t,  "unsigned 8-bit");
TYPE_NAME(uint16_t, "unsigned 16-bit");
TYPE_NAME(uint32_t, "unsigned 32-bit");
TYPE_NAME(uint64_t, "unsigned 64-bit");
TYPE_NAME(int8_t,   "signed 8-bit");
TYPE_NAME(int16_t,  "signed 16-bit");
TYPE_NAME(int32_t,  "signed 32-bit");
TYPE_NAME(int64_t,  "signed 64-bit");

#undef TYPE_NAME
