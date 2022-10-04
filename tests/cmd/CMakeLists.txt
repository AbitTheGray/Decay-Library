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
configure_file(test_img.png ${CMAKE_BINARY_DIR}/tests/cmd COPYONLY)
add_test(
    NAME Test_CMD_wad_add
    COMMAND DecayLib_Command
        wad_add
        --file test.wad
        --image test_img.png
        test.png
)
set_tests_properties(Test_CMD_wad_add PROPERTIES LABELS "cmd;GoldSrc;wad;wad3")

# wad
configure_file(test_wad.wad ${CMAKE_BINARY_DIR}/tests/cmd COPYONLY)
add_test(
    NAME Test_CMD_wad
    COMMAND DecayLib_Command
        wad
        --file test_wad.wad
        --info
        --dump test_wad
)
set_tests_properties(Test_CMD_wad PROPERTIES LABELS "cmd;GoldSrc;wad;wad3")

# rmf2map (GoldSrc)
configure_file(test_rmf2map.rmf ${CMAKE_BINARY_DIR}/tests/cmd COPYONLY)
add_test(
    NAME Test_CMD_rmf2map
    COMMAND DecayLib_Command
        rmf2map
        --file test_rmf2map.rmf
        --map test_rmf2map_goldsrc.map
)
set_tests_properties(Test_CMD_rmf2map PROPERTIES LABELS "cmd;GoldSrc;rmf;map")

# rmf2map (IdTech2)
add_test(
    NAME Test_CMD_rmf2map_idtech2
    COMMAND DecayLib_Command
        rmf2map
        --file test_rmf2map.rmf
        --map test_rmf2map_idtech2.map
        --idtech2
)
set_tests_properties(Test_CMD_rmf2map_idtech2 PROPERTIES LABELS "cmd;GoldSrc;IdTech2;rmf;map")

# map2rmf
configure_file(test_map2rmf.map ${CMAKE_BINARY_DIR}/tests/cmd COPYONLY)
add_test(
    NAME Test_CMD_map2rmf
    COMMAND DecayLib_Command
        map2rmf
        --file test_map2rmf.map
        --rmf test_map2rmf.rmf
)
set_tests_properties(Test_CMD_map2rmf PROPERTIES LABELS "cmd;GoldSrc;rmf;map")

# fgd
configure_file(test_fgd.fgd ${CMAKE_BINARY_DIR}/tests/cmd COPYONLY)
add_test(
    NAME Test_CMD_fgd
    COMMAND DecayLib_Command
        fgd
        --file test_fgd.fgd
        --add test_entity.fgd
        --output test_fgd_out.fgd
        --output_json test_fgd_out.json
)
set_tests_properties(Test_CMD_fgd PROPERTIES LABELS "cmd;GoldSrc;Source;fgd")
