add_executable(Test_Fgd_TestFile main.cpp)

target_link_libraries(Test_Fgd_TestFile DecayLib)

configure_file(file.fgd ${CMAKE_BINARY_DIR}/tests/fgd_testfile COPYONLY)
configure_file(base.fgd ${CMAKE_BINARY_DIR}/tests/fgd_testfile COPYONLY)

add_test(NAME Test_Fgd_TestFile COMMAND Test_Fgd_TestFile ${CMAKE_BINARY_DIR}/tests/fgd_testfile/file.fgd)
set_tests_properties(Test_Fgd_TestFile PROPERTIES LABELS "GoldSrc;fgd")

add_test(NAME Test_Fgd_TestFile2 COMMAND Test_Fgd_TestFile ${CMAKE_BINARY_DIR}/tests/fgd_testfile/base.fgd)
set_tests_properties(Test_Fgd_TestFile2 PROPERTIES LABELS "GoldSrc;fgd")
