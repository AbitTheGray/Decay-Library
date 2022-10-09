add_executable(Test_Mdl_Obj main.cpp)

target_link_libraries(Test_Mdl_Obj DecayLib)

add_test(NAME Test_Mdl_Obj_archer COMMAND Test_Mdl_Obj ${PROJECT_SOURCE_DIR}/half-life/valve/models/archer.mdl)
set_tests_properties(Test_Mdl_Obj_archer PROPERTIES LABELS "GoldSrc;mdl")

add_test(NAME Test_Mdl_Obj_mbarrel COMMAND Test_Mdl_Obj ${PROJECT_SOURCE_DIR}/half-life/valve/models/mbarrel.mdl)
set_tests_properties(Test_Mdl_Obj_mbarrel PROPERTIES LABELS "GoldSrc;mdl")

add_test(NAME Test_Mdl_Obj_baby_headcrab COMMAND Test_Mdl_Obj ${PROJECT_SOURCE_DIR}/half-life/valve/models/baby_headcrab.mdl)
set_tests_properties(Test_Mdl_Obj_baby_headcrab PROPERTIES LABELS "GoldSrc;mdl")
