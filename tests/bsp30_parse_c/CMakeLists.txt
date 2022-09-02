add_executable(Test_Bsp30_Parse_C main.c)

target_link_libraries(Test_Bsp30_Parse_C DecayLib_API)

add_test(NAME Test_Bsp30_Parse_C COMMAND Test_Bsp30_Parse_C)
