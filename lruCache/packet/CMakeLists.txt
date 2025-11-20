cmake_minimum_required(VERSION 3.10)
project(lru_packet_cache C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(SOURCES
    hash.c
    lru_packet_cache.c
    main.c
)

add_executable(lru_test ${SOURCES})
target_include_directories(lru_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# optional compile-time overrides
if(DEFINED CAPACITY)
  target_compile_definitions(lru_test PRIVATE CAPACITY=${CAPACITY})
endif()
if(DEFINED MAX_PACKET_SIZE)
  target_compile_definitions(lru_test PRIVATE MAX_PACKET_SIZE=${MAX_PACKET_SIZE})
endif()
if(DEFINED HASH_SIZE)
  target_compile_definitions(lru_test PRIVATE HASH_SIZE=${HASH_SIZE})
endif()

if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(lru_test PRIVATE -Wall -Wextra)
endif()
