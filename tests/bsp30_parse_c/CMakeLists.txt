add_executable(Test_Bsp30_Parse_C main.c)

target_link_libraries(Test_Bsp30_Parse_C DecayLib_API)

add_test(NAME Test_Bsp30_Parse_C COMMAND Test_Bsp30_Parse_C)
set_tests_properties(Test_Bsp30_Parse_C PROPERTIES LABELS "GoldSrc;bsp;bsp30;C")
