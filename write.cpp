#include <write.h>

template<> const char* type_name<uint8_t>::value = "unsigned 8-bit";
template<> const char* type_name<uint16_t>::value = "unsigned 16-bit";
template<> const char* type_name<uint32_t>::value = "unsigned 32-bit";
template<> const char* type_name<uint64_t>::value = "unsigned 64-bit";
template<> const char* type_name<int8_t>::value = "signed 8-bit";
template<> const char* type_name<int16_t>::value = "signed 16-bit";
template<> const char* type_name<int32_t>::value = "signed 32-bit";
template<> const char* type_name<int64_t>::value = "signed 64-bit";
