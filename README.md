# C++ RTTI TypeInfo Bug Reproducer

This is a minimal reproducer for a subtle C++ RTTI (Run-Time Type Information) bug that occurs when using `std::type_index` comparisons across shared library boundaries.

## The Problem

When a type is used as a template parameter in two different shared libraries,
the `std::type_info` objects for the same type may not be unified by the dynamic linker, causing `std::type_index` comparisons to fail even though the types are identical.

## Files

- `simple_type.hpp` - A simple non-template class (mimics `MetadataFlag`)
- `simple_params.hpp` - A template-based container with runtime type checking (mimics `Params`)
- `libstore.cpp` - Shared library that stores a value (compiled with `-fvisibility=default`)
- `libfetch.cpp` - Shared library that fetches a value (compiled with `-fvisibility=hidden`)
- `main.cpp` - Executable that demonstrates the bug
- `CMakeLists.txt` - Build configuration

## Building

```bash
mkdir build
cd build

# Build without the workaround (demonstrates the bug)
cmake ..
cmake --build .

# Or build with the strcmp workaround
cmake .. -DUSE_STRCMP_FIX=ON
cmake --build .
```

## Running

### Without the workaround (should fail):
```bash
./test_typeinfo
```

Expected output on macOS (and possibly Linux):
```
=== C++ RTTI TypeInfo Reproducer ===
Build mode: WITHOUT strcmp workaround (original buggy code)

Step 1: Calling store_value() from libstore.so (default visibility)...
  ✓ Stored SimpleFlag(42) successfully

Step 2: Calling fetch_value() from libfetch.so (hidden visibility)...
  ✗ FAILURE: WRONG TYPE FOR KEY 'my_flag' (stored: N10SimpleFlagE, requested: N10SimpleFlagE)

=== TEST FAILED ===
```

### With the workaround (should pass):
```bash
./test_typeinfo_fixed
```

Expected output:
```
=== C++ RTTI TypeInfo Reproducer ===
Build mode: WITH strcmp workaround

Step 1: Calling store_value() from libstore.so (default visibility)...
  ✓ Stored SimpleFlag(42) successfully

Step 2: Calling fetch_value() from libfetch.so (hidden visibility)...
  ✓ SUCCESS: Retrieved value = 42

=== TEST PASSED ===
```

### Run both tests automatically:
```bash
ctest -V
```


## The Workaround

The workaround uses `strcmp()` on the mangled type names as a fallback:

```cpp
// Original (fails):
if (types_.at(key) != std::type_index(typeid(T))) {
    throw std::runtime_error("Type mismatch!");
}

// Workaround (works):
if (types_.at(key) != std::type_index(typeid(T)) &&
    std::strcmp(types_.at(key).name(), std::type_index(typeid(T)).name()) != 0) {
    throw std::runtime_error("Type mismatch!");
}
```

This works because:
- `std::type_index::operator==` compares typeinfo object addresses/hashes
- `std::type_info::name()` returns a `const char*` mangled name
- Using `strcmp()` compares the actual string content, which is guaranteed to be identical for the same type
- The fast path (`==`) still works when symbols are properly unified

## Platform and Compiler Test Results

### Summary Table

| Platform | Compiler | libstore typeinfo | libfetch typeinfo | Bug Reproduces? |
|----------|----------|-------------------|-------------------|-----------------|
| macOS    | AppleClang 15 | `s` (local) | `s` (local) | **YES ✗** |
| macOS    | GCC 13.3 | `S` (global) | `s` (local) | NO ✓ |
| Linux    | GCC 13.3 | `V` (weak) | (hidden) | NO ✓ |
| Linux    | Clang 18.1 | `V` (weak) | (hidden) | NO ✓ |

