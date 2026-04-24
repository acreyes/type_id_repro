#include "simple_params.hpp"
#include "simple_type.hpp"

#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#elif defined(__GNUC__)
#define EXPORT __attribute__((visibility("default")))
#else
#define EXPORT
#endif

EXPORT void store_value(SimpleParams &params) {
  params.Add("my_flag", SimpleFlag(42));
}

SimpleFlag::SimpleFlag(int v) : value_(v) {}
int SimpleFlag::value() const { return value_; }
