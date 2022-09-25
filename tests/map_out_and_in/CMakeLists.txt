add_executable(Test_Map_Out_and_In main.cpp)

target_link_libraries(Test_Map_Out_and_In DecayLib)

add_test(NAME Test_Map_Out_and_In COMMAND Test_Map_Out_and_In)
set_tests_properties(Test_Map_Out_and_In PROPERTIES LABELS "GoldSrc;IdTech2;map")
