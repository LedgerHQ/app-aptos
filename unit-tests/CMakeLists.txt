cmake_minimum_required(VERSION 3.10)

if(${CMAKE_VERSION} VERSION_LESS 3.10)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

# project information
project(unit_tests
        VERSION 0.1
	      DESCRIPTION "Unit tests for Ledger Nano application"
        LANGUAGES C)


# guard against bad build-type strings
if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug")
endif()

include(CTest)
ENABLE_TESTING()

# specify C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall -pedantic -g -O0 --coverage")

set(GCC_COVERAGE_LINK_FLAGS "--coverage -lgcov")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}")

# guard against in-source builds
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there. You may need to remove CMakeCache.txt. ")
endif()

add_compile_definitions(TEST)

include_directories(../src)
include_directories($ENV{BOLOS_SDK}/lib_standard_app)

add_executable(test_bcs test_bcs.c)
add_executable(test_tx_parser test_tx_parser.c)
add_executable(test_tx_utils test_tx_utils.c)

add_library(base58 SHARED $ENV{BOLOS_SDK}/lib_standard_app/base58.c)
add_library(bip32 SHARED $ENV{BOLOS_SDK}/lib_standard_app/bip32.c)
add_library(buffer SHARED $ENV{BOLOS_SDK}/lib_standard_app/buffer.c)
add_library(read SHARED $ENV{BOLOS_SDK}/lib_standard_app/read.c)
add_library(write SHARED $ENV{BOLOS_SDK}/lib_standard_app/write.c)
add_library(format SHARED $ENV{BOLOS_SDK}/lib_standard_app/format.c)
add_library(varint SHARED $ENV{BOLOS_SDK}/lib_standard_app/varint.c)
add_library(apdu_parser SHARED $ENV{BOLOS_SDK}/lib_standard_app/parser.c)
add_library(bcs SHARED ../src/bcs/init.c ../src/bcs/decoder.c ../src/bcs/utf8.c)
add_library(transaction_deserialize ../src/transaction/deserialize.c)
add_library(transaction_utils ../src/transaction/utils.c)

target_link_libraries(test_bcs PUBLIC cmocka gcov bcs buffer bip32 varint write read)
target_link_libraries(test_tx_parser PUBLIC
                      transaction_deserialize
                      bcs
                      buffer
                      bip32
                      cmocka
                      gcov
                      varint
                      write
                      read
                      transaction_utils)
target_link_libraries(test_tx_utils PUBLIC
                      cmocka
                      gcov
                      transaction_utils)

add_test(test_bcs test_bcs)
add_test(test_tx_parser test_tx_parser)
add_test(test_tx_utils test_tx_utils)
