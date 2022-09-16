add_executable(Test_Map_TestFile main.cpp)

target_link_libraries(Test_Map_TestFile DecayLib)

configure_file(test.map ${CMAKE_BINARY_DIR}/tests/map_testfile COPYONLY)

add_test(NAME Test_Map_TestFile COMMAND Test_Map_TestFile test.map)
set_tests_properties(Test_Map_TestFile PROPERTIES LABELS "GoldSrc;Quake;fgd")
