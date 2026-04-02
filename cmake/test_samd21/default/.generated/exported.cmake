set(DEPENDENT_MP_BIN2HEXtest_samd21_default_5irDnAg8 "c:/Program Files/Microchip/xc32/v5.00/bin/xc32-bin2hex.exe")
set(DEPENDENT_DEPENDENT_TARGET_ELFtest_samd21_default_5irDnAg8 ${CMAKE_CURRENT_LIST_DIR}/../../../../out/test_samd21/default.elf)
set(DEPENDENT_TARGET_DIRtest_samd21_default_5irDnAg8 ${CMAKE_CURRENT_LIST_DIR}/../../../../out/test_samd21)
set(DEPENDENT_BYPRODUCTStest_samd21_default_5irDnAg8 ${DEPENDENT_TARGET_DIRtest_samd21_default_5irDnAg8}/${sourceFileNametest_samd21_default_5irDnAg8}.c)
add_custom_command(
    OUTPUT ${DEPENDENT_TARGET_DIRtest_samd21_default_5irDnAg8}/${sourceFileNametest_samd21_default_5irDnAg8}.c
    COMMAND ${DEPENDENT_MP_BIN2HEXtest_samd21_default_5irDnAg8} --image ${DEPENDENT_DEPENDENT_TARGET_ELFtest_samd21_default_5irDnAg8} --image-generated-c ${sourceFileNametest_samd21_default_5irDnAg8}.c --image-generated-h ${sourceFileNametest_samd21_default_5irDnAg8}.h --image-copy-mode ${modetest_samd21_default_5irDnAg8} --image-offset ${addresstest_samd21_default_5irDnAg8} 
    WORKING_DIRECTORY ${DEPENDENT_TARGET_DIRtest_samd21_default_5irDnAg8}
    DEPENDS ${DEPENDENT_DEPENDENT_TARGET_ELFtest_samd21_default_5irDnAg8})
add_custom_target(
    dependent_produced_source_artifacttest_samd21_default_5irDnAg8 
    DEPENDS ${DEPENDENT_TARGET_DIRtest_samd21_default_5irDnAg8}/${sourceFileNametest_samd21_default_5irDnAg8}.c
    )
