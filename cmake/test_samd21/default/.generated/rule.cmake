# The following functions contains all the flags passed to the different build stages.

set(PACK_REPO_PATH "C:/Users/insoo/.mchp_packs" CACHE PATH "Path to the root of a pack repository.")

function(test_samd21_default_default_XC32_assemble_rule target)
    set(options
        "-g"
        "${ASSEMBLER_PRE}"
        "-mprocessor=ATSAMD21J18A"
        "-Wa,--defsym=__MPLAB_BUILD=1${MP_EXTRA_AS_POST},--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,--gdwarf-2,-I${CMAKE_CURRENT_SOURCE_DIR}/../../../firmware/test_samd21.X"
        "-mdfp=${PACK_REPO_PATH}/Microchip/SAMD21_DFP/3.4.116/samd21a")
    list(REMOVE_ITEM options "")
    target_compile_options(${target} PRIVATE "${options}")
    target_compile_definitions(${target} PRIVATE "__DEBUG=1")
    target_include_directories(${target} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../firmware/test_samd21.X")
endfunction()
function(test_samd21_default_default_XC32_assembleWithPreprocess_rule target)
    set(options
        "-x"
        "assembler-with-cpp"
        "-g"
        "${MP_EXTRA_AS_PRE}"
        "${DEBUGGER_NAME_AS_MACRO}"
        "-mdfp=${PACK_REPO_PATH}/Microchip/SAMD21_DFP/3.4.116/samd21a"
        "-mprocessor=ATSAMD21J18A"
        "-Wa,--defsym=__MPLAB_BUILD=1${MP_EXTRA_AS_POST},--defsym=__MPLAB_DEBUG=1,--gdwarf-2,--defsym=__DEBUG=1,-I${CMAKE_CURRENT_SOURCE_DIR}/../../../firmware/test_samd21.X")
    list(REMOVE_ITEM options "")
    target_compile_options(${target} PRIVATE "${options}")
    target_compile_definitions(${target}
        PRIVATE "__DEBUG=1"
        PRIVATE "XPRJ_default=default")
    target_include_directories(${target} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../firmware/test_samd21.X")
endfunction()
function(test_samd21_default_default_XC32_compile_rule target)
    set(options
        "-g"
        "${CC_PRE}"
        "-x"
        "c"
        "-c"
        "-mprocessor=ATSAMD21J18A"
        "-ffunction-sections"
        "-fdata-sections"
        "-O1"
        "-Werror"
        "-Wall"
        "-mdfp=${PACK_REPO_PATH}/Microchip/SAMD21_DFP/3.4.116/samd21a")
    list(REMOVE_ITEM options "")
    target_compile_options(${target} PRIVATE "${options}")
    target_compile_definitions(${target}
        PRIVATE "__DEBUG"
        PRIVATE "XPRJ_default=default")
    target_include_directories(${target}
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../firmware/src"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../firmware/src/config/default"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../firmware/src/packs/ATSAMD21J18A_DFP"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../firmware/src/packs/CMSIS"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../firmware/src/packs/CMSIS/CMSIS/Core/Include"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../firmware/test_samd21.X"
        PRIVATE "${PACK_REPO_PATH}/ARM/CMSIS/5.4.0/CMSIS/Core/Include")
endfunction()
function(test_samd21_default_default_XC32_compile_cpp_rule target)
    set(options
        "-g"
        "${CC_PRE}"
        "${DEBUGGER_NAME_AS_MACRO}"
        "-mprocessor=ATSAMD21J18A"
        "-frtti"
        "-fexceptions"
        "-fno-check-new"
        "-fenforce-eh-specs"
        "-ffunction-sections"
        "-O1"
        "-fno-common"
        "-mdfp=${PACK_REPO_PATH}/Microchip/SAMD21_DFP/3.4.116/samd21a")
    list(REMOVE_ITEM options "")
    target_compile_options(${target} PRIVATE "${options}")
    target_compile_definitions(${target}
        PRIVATE "__DEBUG"
        PRIVATE "XPRJ_default=default")
    target_include_directories(${target}
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../firmware/src"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../firmware/src/config/default"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../firmware/src/packs/ATSAMD21J18A_DFP"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../firmware/src/packs/CMSIS"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../firmware/src/packs/CMSIS/CMSIS/Core/Include"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../firmware/test_samd21.X"
        PRIVATE "${PACK_REPO_PATH}/ARM/CMSIS/5.4.0/CMSIS/Core/Include")
endfunction()
function(test_samd21_default_dependentObject_rule target)
    set(options
        "-mprocessor=ATSAMD21J18A"
        "-mdfp=${PACK_REPO_PATH}/Microchip/SAMD21_DFP/3.4.116/samd21a")
    list(REMOVE_ITEM options "")
    target_compile_options(${target} PRIVATE "${options}")
endfunction()
function(test_samd21_default_link_rule target)
    set(options
        "-g"
        "${MP_EXTRA_LD_PRE}"
        "${DEBUGGER_OPTION_TO_LINKER}"
        "${DEBUGGER_NAME_AS_MACRO}"
        "-mprocessor=ATSAMD21J18A"
        "-mno-device-startup-code"
        "-Wl,--defsym=__MPLAB_BUILD=1${MP_EXTRA_LD_POST},--script=${test_samd21_default_LINKER_SCRIPT},--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,--defsym=_min_heap_size=512,--gc-sections,-L${CMAKE_CURRENT_SOURCE_DIR}/../../../firmware/test_samd21.X,-Map=mem.map,--memorysummary,memoryfile.xml"
        "-mdfp=${PACK_REPO_PATH}/Microchip/SAMD21_DFP/3.4.116/samd21a")
    list(REMOVE_ITEM options "")
    target_link_options(${target} PRIVATE "${options}")
    target_compile_definitions(${target} PRIVATE "XPRJ_default=default")
endfunction()
function(test_samd21_default_bin2hex_rule target)
    add_custom_target(
        test_samd21_default_Bin2Hex ALL
        COMMAND ${MP_BIN2HEX} ${test_samd21_default_image_name}
        WORKING_DIRECTORY ${test_samd21_default_output_dir}
        BYPRODUCTS "${test_samd21_default_output_dir}/${test_samd21_default_image_base_name}.hex"
        COMMENT "Convert build file to .hex")
    add_dependencies(test_samd21_default_Bin2Hex ${target})
endfunction()
