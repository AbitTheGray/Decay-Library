add_executable(Test_WadParse_All main.cpp)

target_link_libraries(Test_WadParse_All DecayLib)

add_test(NAME Test_WadParse_All COMMAND Test_WadParse_All)
