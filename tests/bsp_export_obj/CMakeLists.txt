add_executable(Test_Bsp_ExportObj main.cpp)

target_link_libraries(Test_Bsp_ExportObj DecayLib)

add_test(NAME Test_Bsp_ExportObj COMMAND Test_Bsp_ExportObj)
