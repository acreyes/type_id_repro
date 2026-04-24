#include "simple_params.hpp"
#include "simple_type.hpp"

#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#elif defined(__GNUC__)
#define EXPORT __attribute__((visibility("default")))
#else
#define EXPORT
#endif

EXPORT SimpleFlag fetch_value(SimpleParams &params) {
  return params.Get<SimpleFlag>("my_flag");
}