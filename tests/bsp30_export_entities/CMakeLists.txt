add_executable(Text_Bsp30_ExportEntities main.cpp)

target_link_libraries(Text_Bsp30_ExportEntities DecayLib)

add_test(NAME Text_Bsp30_ExportEntities COMMAND Text_Bsp30_ExportEntities)
set_tests_properties(Text_Bsp30_ExportEntities PROPERTIES LABELS "GoldSrc;bsp;bsp30;json")
