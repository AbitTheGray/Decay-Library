add_executable(Text_Bsp30_ExportObj main.cpp)

target_link_libraries(Text_Bsp30_ExportObj DecayLib)

add_test(NAME Text_Bsp30_ExportObj COMMAND Text_Bsp30_ExportObj)
set_tests_properties(Text_Bsp30_ExportObj PROPERTIES LABELS "GoldSrc;bsp;bsp30")
