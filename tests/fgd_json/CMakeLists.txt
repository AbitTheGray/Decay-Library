if(DECAY_JSON_LIB)
    add_executable(Test_Fgd_Json main.cpp)

    target_link_libraries(Test_Fgd_Json DecayLib)

    configure_file(file.fgd ${CMAKE_BINARY_DIR}/tests/fgd_json COPYONLY)

    add_test(NAME Test_Fgd_Json COMMAND Test_Fgd_Json)
    set_tests_properties(Test_Fgd_Json PROPERTIES LABELS "GoldSrc;Source;fgd;json")
endif()
