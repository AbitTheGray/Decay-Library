add_executable(Test_Wad3_Out_and_In main.cpp)

target_link_libraries(Test_Wad3_Out_and_In DecayLib)

add_test(NAME Test_Wad3_Out_and_In COMMAND Test_Wad3_Out_and_In)
set_tests_properties(Test_Wad3_Out_and_In PROPERTIES LABELS "GoldSrc;wad;wad3")
