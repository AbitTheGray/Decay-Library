add_executable(Test_BspParse main.cpp)

target_link_libraries(Test_BspParse DecayLib)

add_test(NAME Test_BspParse COMMAND Test_BspParse)
