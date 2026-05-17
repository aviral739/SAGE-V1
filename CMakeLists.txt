cmake_minimum_required(VERSION 3.20)
project(SAGE VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall -Wextra)

option(SAGE_ENABLE_AVX2 "Enable AVX2 SIMD acceleration for search" ON)

if(SAGE_ENABLE_AVX2)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        add_compile_options(-mavx2)
    endif()
endif()

include_directories(engine/include)

set(SOURCES
    main.cpp
    engine/src/dataset_generator.cpp
    engine/src/search.cpp
    engine/src/hardware.cpp
    engine/src/benchmark.cpp
    engine/src/resource_policy.cpp
    engine/src/chunk_manager.cpp
    engine/src/metadata.cpp
    engine/src/strategy.cpp
    engine/src/csv_export.cpp
    engine/src/thread_pool.cpp
)

add_executable(sage ${SOURCES})
target_compile_features(sage PRIVATE cxx_std_20)

find_package(Threads REQUIRED)
target_link_libraries(sage PRIVATE Threads::Threads)
