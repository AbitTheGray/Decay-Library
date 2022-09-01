add_executable(Test_WadParse_C main.cpp)

target_link_libraries(Test_WadParse_C DecayLib_API)

add_test(NAME Test_WadParse_C COMMAND Test_WadParse_C)
