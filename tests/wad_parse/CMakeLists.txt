add_executable(Test_WadParse main.cpp)

target_link_libraries(Test_WadParse DecayLib)

add_test(NAME Test_WadParse COMMAND Test_WadParse)
