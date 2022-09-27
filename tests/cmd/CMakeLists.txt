# bsp2obj
add_test(
    NAME Test_CMD_bsp2obj
    COMMAND DecayLib_Command
        bsp2obj
        --file "${PROJECT_SOURCE_DIR}/half-life/cstrike/maps/de_dust2.bsp"
        --obj de_dust2.obj
        --mtl de_dust2.mtl
        --textures de_dust2
)
set_tests_properties(Test_CMD_bsp2obj PROPERTIES LABELS "cmd;GoldSrc;bsp;bsp30")

# bsp2wad
add_test(
    NAME Test_CMD_bsp2wad
    COMMAND DecayLib_Command
        bsp2wad
        --file "${PROJECT_SOURCE_DIR}/half-life/cstrike/maps/de_dust2.bsp"
        --wad de_dust2.wad
        --newbsp de_dust2.notexture.bsp
)
set_tests_properties(Test_CMD_bsp2wad PROPERTIES LABELS "cmd;GoldSrc;bsp;bsp30")

# bsp_lightmap
add_test(
    NAME Test_CMD_bsp_lightmap
    COMMAND DecayLib_Command
        bsp_lightmap
        --file "${PROJECT_SOURCE_DIR}/half-life/cstrike/maps/de_dust2.bsp"
        --lightmap de_dust2_lightmap.png
)
set_tests_properties(Test_CMD_bsp_lightmap PROPERTIES LABELS "cmd;GoldSrc;bsp;bsp30")

# bsp_entity
configure_file(test_entity.bsp ${CMAKE_BINARY_DIR}/tests/cmd COPYONLY)
configure_file(test_entity.fgd ${CMAKE_BINARY_DIR}/tests/cmd COPYONLY)
configure_file(test_entity.kv ${CMAKE_BINARY_DIR}/tests/cmd COPYONLY)
configure_file(test_entity.json ${CMAKE_BINARY_DIR}/tests/cmd COPYONLY)
add_test(
    NAME Test_CMD_bsp_entity
    COMMAND DecayLib_Command
        bsp_entity
        --file test_entity.bsp
        --validate test_entity.fgd
        --extract test_entity.out.kv
        --extract_json test_entity.out.json
        --outbsp test_entity.out.bsp
)
set_tests_properties(Test_CMD_bsp_entity PROPERTIES LABELS "cmd;GoldSrc;bsp;bsp30")

# bsp_entity (dump map)
add_test(
    NAME Test_CMD_bsp_entity_dump
    COMMAND DecayLib_Command
        bsp_entity
        --file "${PROJECT_SOURCE_DIR}/half-life/cstrike/maps/de_dust2.bsp"
        --extract de_dust2_entites.kv
        --extract_json de_dust2_entites.json
)
set_tests_properties(Test_CMD_bsp_entity_dump PROPERTIES LABELS "cmd;GoldSrc;bsp;bsp30")

# wad_add
configure_file(test.png ${CMAKE_BINARY_DIR}/tests/cmd COPYONLY)
add_test(
    NAME Test_CMD_wad_add
    COMMAND DecayLib_Command
        wad_add
        --file test.wad
        test.png
)
set_tests_properties(Test_CMD_wad_add PROPERTIES LABELS "cmd;GoldSrc;wad;wad3")
