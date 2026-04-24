FROM ubuntu:24.04

# Install build tools - both GCC and Clang
RUN apt-get update && \
    apt-get install -y \
        build-essential \
        cmake \
        clang \
        binutils && \
    rm -rf /var/lib/apt/lists/*

# Copy reproducer source
WORKDIR /test
COPY . /test/

# Test with GCC
RUN echo "========================================" && \
    echo "  Testing with GCC" && \
    echo "========================================" && \
    mkdir -p build-gcc && \
    cd build-gcc && \
    cmake -DCMAKE_CXX_COMPILER=g++ .. && \
    cmake --build . && \
    echo "" && \
    echo "--- GCC: Test WITHOUT fix (expecting FAILURE) ---" && \
    ./test_typeinfo || echo "Failed as expected" && \
    echo "" && \
    echo "--- GCC: Test WITH fix (expecting SUCCESS) ---" && \
    ./test_typeinfo_fixed && \
    echo "" && \
    echo "--- GCC: Symbol inspection ---" && \
    echo "libstore.so symbols:" && \
    nm -gC libstore.so | grep "typeinfo for SimpleFlag" || echo "  (no typeinfo symbol found)" && \
    echo "libfetch.so symbols:" && \
    nm -gC libfetch.so | grep "typeinfo for SimpleFlag" || echo "  (no typeinfo symbol found - hidden as expected)" && \
    cd ..

# Test with Clang
RUN echo "" && \
    echo "========================================" && \
    echo "  Testing with Clang" && \
    echo "========================================" && \
    mkdir -p build-clang && \
    cd build-clang && \
    cmake -DCMAKE_CXX_COMPILER=clang++ .. && \
    cmake --build . && \
    echo "" && \
    echo "--- Clang: Test WITHOUT fix (expecting FAILURE) ---" && \
    ./test_typeinfo || echo "Failed as expected" && \
    echo "" && \
    echo "--- Clang: Test WITH fix (expecting SUCCESS) ---" && \
    ./test_typeinfo_fixed && \
    echo "" && \
    echo "--- Clang: Symbol inspection ---" && \
    echo "libstore.so symbols:" && \
    nm -gC libstore.so | grep "typeinfo for SimpleFlag" || echo "  (no typeinfo symbol found)" && \
    echo "libfetch.so symbols:" && \
    nm -gC libfetch.so | grep "typeinfo for SimpleFlag" || echo "  (no typeinfo symbol found - hidden as expected)"

CMD ["/bin/bash"]
