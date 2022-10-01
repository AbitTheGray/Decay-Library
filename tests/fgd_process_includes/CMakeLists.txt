add_executable(Test_Fgd_Process_Includes main.cpp)

target_link_libraries(Test_Fgd_Process_Includes DecayLib)

configure_file(main.fgd ${CMAKE_BINARY_DIR}/tests/fgd_process_includes COPYONLY)
configure_file(base.fgd ${CMAKE_BINARY_DIR}/tests/fgd_process_includes COPYONLY)
configure_file(base2.fgd ${CMAKE_BINARY_DIR}/tests/fgd_process_includes COPYONLY)

add_test(NAME Test_Fgd_Process_Includes COMMAND Test_Fgd_Process_Includes)
set_tests_properties(Test_Fgd_Process_Includes PROPERTIES LABELS "GoldSrc;Source;fgd")
