#include "simple_params.hpp"
#include "simple_type.hpp"

// This library will be compiled with -fvisibility=hidden
// Mimics Python extension fetching a MetadataFlag from Params

SimpleFlag fetch_value(SimpleParams &params) {
  // This instantiates SimpleParams::Get<SimpleFlag>() in this compilation unit
  // The typeinfo for SimpleFlag will be generated here again
  // BUG: With hidden visibility, this typeinfo won't match the one in libstore!
  return params.Get<SimpleFlag>("my_flag");
}
