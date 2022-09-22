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
