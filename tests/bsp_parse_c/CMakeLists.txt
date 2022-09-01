add_executable(Test_BspParse_C main.c)

target_link_libraries(Test_BspParse_C DecayLib_API)

add_test(NAME Test_BspParse_C COMMAND Test_BspParse_C)
