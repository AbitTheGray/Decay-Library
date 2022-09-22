# help
add_test(
    NAME Test_CMD_help
    COMMAND DecayLib_Command
        help
)
set_tests_properties(Test_CMD_help PROPERTIES LABELS "cmd")

# bsp2obj
add_test(
    NAME Test_CMD_bsp2obj
    COMMAND DecayLib_Command
        bsp2obj
        "${PROJECT_SOURCE_DIR}/half-life/cstrike/maps/de_dust2.bsp"
        de_dust2.obj
        de_dust2.mtl
        de_dust2
)
set_tests_properties(Test_CMD_bsp2obj PROPERTIES LABELS "cmd;GoldSrc;bsp;bsp30")

# bsp_lightmap
add_test(
    NAME Test_CMD_bsp_lightmap
    COMMAND DecayLib_Command
        bsp_lightmap
        "${PROJECT_SOURCE_DIR}/half-life/cstrike/maps/de_dust2.bsp"
        de_dust2_lightmap.png
)
set_tests_properties(Test_CMD_bsp_lightmap PROPERTIES LABELS "cmd;GoldSrc;bsp;bsp30")

# bsp2wad
add_test(
    NAME Test_CMD_bsp2wad
    COMMAND DecayLib_Command
        bsp2wad
        "${PROJECT_SOURCE_DIR}/half-life/cstrike/maps/de_dust2.bsp"
        de_dust2.wad
        de_dust2.notexture.bsp
)
set_tests_properties(Test_CMD_bsp2wad PROPERTIES LABELS "cmd;GoldSrc;bsp;bsp30")
