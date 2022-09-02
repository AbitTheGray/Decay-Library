add_executable(Text_Wad3_Image main.cpp)

target_link_libraries(Text_Wad3_Image DecayLib)

add_test(NAME Text_Wad3_Image COMMAND Text_Wad3_Image)
