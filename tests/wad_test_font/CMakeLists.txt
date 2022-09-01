add_executable(Test_Wad_Font main.cpp)

target_link_libraries(Test_Wad_Font DecayLib)

add_test(NAME Test_Wad_Font COMMAND Test_Wad_Font)
