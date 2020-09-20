cmake_minimum_required(VERSION 3.5)

project(eosio_contracts)

set(VERSION_MAJOR 3)
set(VERSION_MINOR 2)
set(VERSION_PATCH 0)
set(VERSION_SUFFIX dev)

if(VERSION_SUFFIX)
  set(VERSION_FULL "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}-${VERSION_SUFFIX}")
else()
  set(VERSION_FULL "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
endif()

message(STATUS "Building reference-contracts v${VERSION_FULL}")

include(ExternalProject)

find_package(cdt)

option(SYSTEM_CONFIGURABLE_WASM_LIMITS
       "Enables use of the host functions activated by the CONFIGURABLE_WASM_LIMITS p