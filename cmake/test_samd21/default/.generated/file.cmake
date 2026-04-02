# The following variables contains the files used by the different stages of the build process.
set(test_samd21_default_default_XC32_FILE_TYPE_assemble)
set_source_files_properties(${test_samd21_default_default_XC32_FILE_TYPE_assemble} PROPERTIES LANGUAGE ASM)

# For assembly files, add "." to the include path for each file so that .include with a relative path works
foreach(source_file ${test_samd21_default_default_XC32_FILE_TYPE_assemble})
        set_source_files_properties(${source_file} PROPERTIES INCLUDE_DIRECTORIES "$<PATH:NORMAL_PATH,$<PATH:REMOVE_FILENAME,${source_file}>>")
endforeach()

set(test_samd21_default_default_XC32_FILE_TYPE_assembleWithPreprocess)
set_source_files_properties(${test_samd21_default_default_XC32_FILE_TYPE_assembleWithPreprocess} PROPERTIES LANGUAGE ASM)

# For assembly files, add "." to the include path for each file so that .include with a relative path works
foreach(source_file ${test_samd21_default_default_XC32_FILE_TYPE_assembleWithPreprocess})
        set_source_files_properties(${source_file} PROPERTIES INCLUDE_DIRECTORIES "$<PATH:NORMAL_PATH,$<PATH:REMOVE_FILENAME,${source_file}>>")
endforeach()

set(test_samd21_default_default_XC32_FILE_TYPE_compile
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../firmware/src/config/default/exceptions.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../firmware/src/config/default/initialization.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../firmware/src/config/default/interrupts.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../firmware/src/config/default/libc_syscalls.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../firmware/src/config/default/peripheral/clock/plib_clock.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../firmware/src/config/default/peripheral/evsys/plib_evsys.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../firmware/src/config/default/peripheral/nvic/plib_nvic.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../firmware/src/config/default/peripheral/nvmctrl/plib_nvmctrl.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../firmware/src/config/default/peripheral/port/plib_port.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../firmware/src/config/default/peripheral/rtc/plib_rtc_timer.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../firmware/src/config/default/peripheral/sercom/usart/plib_sercom3_usart.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../firmware/src/config/default/startup_xc32.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../firmware/src/config/default/stdio/xc32_monitor.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../firmware/src/config/default/touch/touch.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../firmware/src/config/default/touch/touch_example.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../firmware/src/main.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../firmware/src/sml_recognition_run.c")
set_source_files_properties(${test_samd21_default_default_XC32_FILE_TYPE_compile} PROPERTIES LANGUAGE C)
set(test_samd21_default_default_XC32_FILE_TYPE_compile_cpp)
set_source_files_properties(${test_samd21_default_default_XC32_FILE_TYPE_compile_cpp} PROPERTIES LANGUAGE CXX)
set(test_samd21_default_default_XC32_FILE_TYPE_link
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../firmware/src/config/default/touch/lib/qtm_acq_samd21_0x0024.X.a"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../firmware/src/config/default/touch/lib/qtm_freq_hop_auto_cm0p_0x0004.X.a"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../firmware/src/config/default/touch/lib/qtm_freq_hop_cm0p_0x0006.X.a"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../firmware/src/config/default/touch/lib/qtm_touch_key_cm0p_0x0002.X.a"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../firmware/src/sensiml/lib/libsensiml.a")
set(test_samd21_default_default_XC32_FILE_TYPE_bin2hex)

# The linker script used for the build.
set(test_samd21_default_LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/../../../firmware/src/config/default/ATSAMD21J18A.ld")
set(test_samd21_default_image_name "default.elf")
set(test_samd21_default_image_base_name "default")

# The output directory of the final image.
set(test_samd21_default_output_dir "${CMAKE_CURRENT_SOURCE_DIR}/../../../out/test_samd21")

# The full path to the final image.
set(test_samd21_default_full_path_to_image ${test_samd21_default_output_dir}/${test_samd21_default_image_name})
