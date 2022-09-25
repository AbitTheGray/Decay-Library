add_executable(Test_Rmf_TestFile main.cpp)

target_link_libraries(Test_Rmf_TestFile DecayLib)

configure_file(test.rmf ${CMAKE_BINARY_DIR}/tests/rmf_testfile COPYONLY)

add_test(NAME Test_Rmf_TestFile COMMAND Test_Rmf_TestFile test.rmf)
set_tests_properties(Test_Rmf_TestFile PROPERTIES LABELS "GoldSrc;rmf")
