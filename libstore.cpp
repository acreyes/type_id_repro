#include "simple_params.hpp"
#include "simple_type.hpp"

// This library will be compiled with -fvisibility=default
// Mimics libparthenon.dylib storing a MetadataFlag in Params

#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __attribute__((visibility("default")))
#endif

EXPORT void store_value(SimpleParams &params) {
  // This instantiates SimpleParams::Add<SimpleFlag>() in this compilation unit
  // The typeinfo for SimpleFlag will be generated here
  params.Add("my_flag", SimpleFlag(42));
}
