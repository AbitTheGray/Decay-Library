add_executable(Test_Wad3_Parse main.cpp)

target_link_libraries(Test_Wad3_Parse DecayLib)

add_test(NAME Test_Wad3_Parse COMMAND Test_Wad3_Parse)
set_tests_properties(Test_Wad3_Parse PROPERTIES LABELS "GoldSrc;wad;wad3")
