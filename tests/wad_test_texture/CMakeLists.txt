add_executable(Test_Wad_Texture main.cpp)

target_link_libraries(Test_Wad_Texture DecayLib)

add_test(NAME Test_Wad_Texture COMMAND Test_Wad_Texture)
