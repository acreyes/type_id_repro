#include "simple_params.hpp"
#include "simple_type.hpp"

// This library will be compiled with -fvisibility=hidden
// Mimics pyKamayan.so (Python extension) fetching a MetadataFlag from Params

#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __attribute__((visibility("default")))
#endif

EXPORT SimpleFlag fetch_value(SimpleParams &params) {
  // This instantiates SimpleParams::Get<SimpleFlag>() in this compilation unit
  // The typeinfo for SimpleFlag will be generated here again
  // BUG: With hidden visibility, this typeinfo won't match the one in libstore!
  return params.Get<SimpleFlag>("my_flag");
}
