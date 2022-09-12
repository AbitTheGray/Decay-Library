add_executable(Test_Fgd_Out_and_In main.cpp)

target_link_libraries(Test_Fgd_Out_and_In DecayLib)

add_test(NAME Test_Fgd_Out_and_In COMMAND Test_Fgd_Out_and_In)
set_tests_properties(Test_Fgd_Out_and_In PROPERTIES LABELS "GoldSrc;fgd")
