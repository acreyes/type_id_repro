#include "simple_params.hpp"
#include "simple_type.hpp"

#include <exception>
#include <iostream>

// External functions from the two shared libraries
void store_value(SimpleParams &params);
SimpleFlag fetch_value(SimpleParams &params);

int main() {
  std::cout << "=== C++ RTTI TypeInfo Reproducer ===\n";
  std::cout << "This demonstrates typeinfo symbol mismatch across library boundaries\n";
  std::cout << "when one library uses -fvisibility=hidden\n\n";

#ifdef USE_STRCMP_FIX
  std::cout << "Build mode: WITH strcmp workaround\n";
#else
  std::cout << "Build mode: WITHOUT strcmp workaround (original buggy code)\n";
#endif
  std::cout << "\n";

  SimpleParams params;

  std::cout << "Step 1: Calling store_value() from libstore.so (default visibility)...\n";
  try {
    store_value(params);
    std::cout << "  ✓ Stored SimpleFlag(42) successfully\n";
  } catch (const std::exception &e) {
    std::cout << "  ✗ UNEXPECTED ERROR: " << e.what() << "\n";
    return 1;
  }

  std::cout
      << "\nStep 2: Calling fetch_value() from libfetch.so (hidden visibility)...\n";
  try {
    auto flag = fetch_value(params);
    std::cout << "  ✓ SUCCESS: Retrieved value = " << flag.value() << "\n";
    std::cout << "\n=== TEST PASSED ===\n";
    return 0;
  } catch (const std::exception &e) {
    std::cout << "  ✗ FAILURE: " << e.what() << "\n";
    std::cout << "\n=== TEST FAILED ===\n";
    std::cout << "\nThis is the bug! The typeinfo for SimpleFlag differs between:\n";
    std::cout << "  - libstore.so (where it was stored)\n";
    std::cout << "  - libfetch.so (where it's being retrieved)\n";
    std::cout << "\nEven though both use the same type, the typeinfo symbols\n";
    std::cout << "are not unified due to -fvisibility=hidden on libfetch.so\n";
    return 1;
  }
}
