add_executable(Test_Wad_Image main.cpp)

target_link_libraries(Test_Wad_Image DecayLib)

add_test(NAME Test_Wad_Image COMMAND Test_Wad_Image)
