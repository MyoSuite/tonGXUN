
add_executable(blockinfo_tester ${CMAKE_CURRENT_SOURCE_DIR}/src/blockinfo_tester.cpp)

target_include_directories(blockinfo_tester PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
                                                   "$<TARGET_PROPERTY:eosio.system,INTERFACE_INCLUDE_DIRECTORIES>")