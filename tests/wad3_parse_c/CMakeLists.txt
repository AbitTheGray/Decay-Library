add_executable(Test_Wad3_Parse_C main.cpp)

target_link_libraries(Test_Wad3_Parse_C DecayLib_API)

add_test(NAME Test_Wad3_Parse_C COMMAND Test_Wad3_Parse_C)
