add_executable(Test_Wad_Out_and_In main.cpp)

target_link_libraries(Test_Wad_Out_and_In DecayLib)

add_test(NAME Test_Wad_Out_and_In COMMAND Test_Wad_Out_and_In)
set_tests_properties(Test_Wad_Out_and_In PROPERTIES LABELS "GoldSrc;wad;wad3")
