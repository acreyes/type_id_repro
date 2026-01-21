# C++ RTTI TypeInfo Bug Reproducer

This is a minimal reproducer for a subtle C++ RTTI (Run-Time Type Information) bug that occurs when using `std::type_index` comparisons across shared library boundaries with different visibility settings.

## The Problem

When a type is used as a template parameter in two different shared libraries:
- One compiled with `-fvisibility=default` 
- One compiled with `-fvisibility=hidden`

The `std::type_info` objects for the same type may not be unified by the dynamic linker, causing `std::type_index` comparisons to fail even though the types are identical.

## Real-World Context

This bug was discovered in the Kamayan/Parthenon project when:
- **libparthenon.dylib** (default visibility) stores a `MetadataFlag` in a `Params` object using `typeid()`
- **pyKamayan.so** (hidden visibility, Python extension via nanobind) tries to retrieve the same `MetadataFlag`
- The type check fails because the typeinfo symbols aren't unified

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

## Testing in Docker (Linux)

To test whether this bug affects Linux (not just macOS), use Docker:

### Quick test with both GCC and Clang:
```bash
cd typeinfo_reproducer
./test_in_docker.sh
```

This will:
1. Build an Ubuntu 24.04 Docker image with GCC and Clang
2. Compile the reproducer with both compilers
3. Run tests to see if the bug reproduces on Linux
4. Display symbol visibility for comparison

### Manual Docker testing:
```bash
# Build the image
docker build -t typeinfo-test .

# Run the tests
docker run --rm typeinfo-test

# Or get an interactive shell to explore
docker run --rm -it typeinfo-test /bin/bash
```

### Expected Results

**If bug reproduces on Linux:**
- Both GCC and Clang will show the "WRONG TYPE" error without the fix
- Confirms the issue is universal (affects all platforms)

**If bug does NOT reproduce on Linux:**
- Tests pass even without the fix
- Suggests macOS-specific behavior (Mach-O vs ELF difference)
- The strcmp workaround is still safe but only strictly necessary on macOS

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

## Debugging Symbol Visibility

On macOS/Linux, you can inspect symbol visibility:

```bash
# Check typeinfo symbols in the libraries
nm -gC libstore.dylib | grep "typeinfo for SimpleFlag"
nm -gC libfetch.dylib | grep "typeinfo for SimpleFlag"

# 'S' = global symbol (exported)
# 's' = local symbol (not exported) - this causes the bug!
```

## Platform and Compiler Test Results

### Summary Table

| Platform | Compiler | libstore typeinfo | libfetch typeinfo | Bug Reproduces? |
|----------|----------|-------------------|-------------------|-----------------|
| macOS    | AppleClang 15 | `s` (local) | `s` (local) | **YES ✗** |
| macOS    | GCC 13.3 | `S` (global) | `s` (local) | NO ✓ |
| Linux    | GCC 13.3 | `V` (weak) | (hidden) | NO ✓ |
| Linux    | Clang 18.1 | `V` (weak) | (hidden) | NO ✓ |

### Key Findings

**The bug ONLY reproduces with AppleClang on macOS!**

This is due to how different compilers handle typeinfo symbol visibility:

#### AppleClang on macOS (Mach-O):
- Marks typeinfo symbols as **local** (`s`) even with `-fvisibility=default`
- Typeinfo for template parameters not explicitly exported remain local
- Mach-O linker doesn't unify local typeinfo symbols across libraries
- **Result: `std::type_index` comparison fails**

#### GCC on macOS (Mach-O):
- Marks typeinfo symbols as **global** (`S`) with `-fvisibility=default`
- Allows Mach-O linker to unify the symbols across libraries
- **Result: Bug does NOT reproduce**

#### Linux (ELF) - Both GCC and Clang:
- More aggressive weak symbol unification via COMDAT groups
- ELF linker resolves typeinfo references even when one library hides the symbol
- **Result: Bug does NOT reproduce**

### Why the strcmp() Fix is Still Important

Even though the bug only affects AppleClang on macOS:
- **Kamayan is built with AppleClang on macOS** (primary development platform)
- The fix is harmless on other platforms (fast path still works)
- Defensive programming against similar linker behavior on other platforms
- Minimal performance overhead (strcmp only runs when fast path fails)

## References

- Parthenon issue: https://github.com/parthenon-hpc-lab/parthenon/pull/1275 (similar typeid issue)
- C++ ABI typeinfo: https://itanium-cxx-abi.github.io/cxx-abi/abi.html#rtti
- GCC visibility: https://gcc.gnu.org/wiki/Visibility

## License

This reproducer is public domain. Use it freely to demonstrate, test, or report this issue.
