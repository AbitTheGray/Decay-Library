add_executable(Test_Bsp30_Parse main.cpp)

target_link_libraries(Test_Bsp30_Parse DecayLib)

add_test(NAME Test_Bsp30_Parse COMMAND Test_Bsp30_Parse)
set_tests_properties(Test_Bsp30_Parse PROPERTIES LABELS "GoldSrc;bsp;bsp30")
