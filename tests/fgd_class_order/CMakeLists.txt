add_executable(Test_Fgd_Class_Order main.cpp)

target_link_libraries(Test_Fgd_Class_Order DecayLib)

add_test(NAME Test_Fgd_Class_Order COMMAND Test_Fgd_Class_Order)
set_tests_properties(Test_Fgd_Class_Order PROPERTIES LABELS "GoldSrc;Source;fgd")
