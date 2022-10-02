# help
add_test(
    NAME Test_CMD_help
    COMMAND DecayLib_Command
        help
)
set_tests_properties(Test_CMD_help PROPERTIES LABELS "cmd")

# bsp2obj
add_test(
    NAME Test_CMD_help_bsp2obj
    COMMAND DecayLib_Command
        help
        bsp2obj
)
set_tests_properties(Test_CMD_help_bsp2obj PROPERTIES LABELS "cmd;cmd_help;GoldSrc;bsp;bsp30")

# bsp_lightmap
add_test(
    NAME Test_CMD_help_bsp_lightmap
    COMMAND DecayLib_Command
        help
        bsp_lightmap
)
set_tests_properties(Test_CMD_help_bsp_lightmap PROPERTIES LABELS "cmd;cmd_help;GoldSrc;bsp;bsp30")

# bsp2wad
add_test(
    NAME Test_CMD_help_bsp2wad
    COMMAND DecayLib_Command
        help
        bsp2wad
)
set_tests_properties(Test_CMD_help_bsp2wad PROPERTIES LABELS "cmd;cmd_help;GoldSrc;bsp;bsp30")

# bsp_entity
add_test(
    NAME Test_CMD_help_bsp_entity
    COMMAND DecayLib_Command
        help
        bsp_entity
)
set_tests_properties(Test_CMD_help_bsp_entity PROPERTIES LABELS "cmd;cmd_help;GoldSrc;bsp;bsp30")

# wad_add
add_test(
    NAME Test_CMD_help_wad_add
    COMMAND DecayLib_Command
        help
        wad_add
)
set_tests_properties(Test_CMD_help_wad_add PROPERTIES LABELS "cmd;cmd_help;GoldSrc;wad;wad3")

# map2rmf
add_test(
    NAME Test_CMD_help_map2rmf
    COMMAND DecayLib_Command
        help
        map2rmf
)
set_tests_properties(Test_CMD_help_map2rmf PROPERTIES LABELS "cmd;cmd_help;GoldSrc;map;rmf")

# rmf2map
add_test(
    NAME Test_CMD_help_rmf2map
    COMMAND DecayLib_Command
        help
        rmf2map
)
set_tests_properties(Test_CMD_help_rmf2map PROPERTIES LABELS "cmd;cmd_help;GoldSrc;map;rmf")

# fgd
add_test(
    NAME Test_CMD_help_fgd
    COMMAND DecayLib_Command
        help
        fgd
)
set_tests_properties(Test_CMD_help_fgd PROPERTIES LABELS "cmd;cmd_help;GoldSrc;Sopurce;fgd")
