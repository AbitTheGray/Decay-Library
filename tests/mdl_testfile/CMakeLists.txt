add_executable(Test_Mdl_TestFile main.cpp)

target_link_libraries(Test_Mdl_TestFile DecayLib)

#configure_file(test.mdl ${CMAKE_BINARY_DIR}/tests/rmf_testfile COPYONLY)

add_test(NAME Test_Mdl_TestFile COMMAND Test_Mdl_TestFile test.mdl)
set_tests_properties(Test_Mdl_TestFile PROPERTIES LABELS "GoldSrc;mdl")
