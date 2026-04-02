include("${CMAKE_CURRENT_LIST_DIR}/rule.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/file.cmake")

set(test_samd21_default_library_list )

# Handle files with suffix s, for group default-XC32
if(test_samd21_default_default_XC32_FILE_TYPE_assemble)
add_library(test_samd21_default_default_XC32_assemble OBJECT ${test_samd21_default_default_XC32_FILE_TYPE_assemble})
    test_samd21_default_default_XC32_assemble_rule(test_samd21_default_default_XC32_assemble)
    list(APPEND test_samd21_default_library_list "$<TARGET_OBJECTS:test_samd21_default_default_XC32_assemble>")

endif()

# Handle files with suffix S, for group default-XC32
if(test_samd21_default_default_XC32_FILE_TYPE_assembleWithPreprocess)
add_library(test_samd21_default_default_XC32_assembleWithPreprocess OBJECT ${test_samd21_default_default_XC32_FILE_TYPE_assembleWithPreprocess})
    test_samd21_default_default_XC32_assembleWithPreprocess_rule(test_samd21_default_default_XC32_assembleWithPreprocess)
    list(APPEND test_samd21_default_library_list "$<TARGET_OBJECTS:test_samd21_default_default_XC32_assembleWithPreprocess>")

endif()

# Handle files with suffix [cC], for group default-XC32
if(test_samd21_default_default_XC32_FILE_TYPE_compile)
add_library(test_samd21_default_default_XC32_compile OBJECT ${test_samd21_default_default_XC32_FILE_TYPE_compile})
    test_samd21_default_default_XC32_compile_rule(test_samd21_default_default_XC32_compile)
    list(APPEND test_samd21_default_library_list "$<TARGET_OBJECTS:test_samd21_default_default_XC32_compile>")

endif()

# Handle files with suffix cpp, for group default-XC32
if(test_samd21_default_default_XC32_FILE_TYPE_compile_cpp)
add_library(test_samd21_default_default_XC32_compile_cpp OBJECT ${test_samd21_default_default_XC32_FILE_TYPE_compile_cpp})
    test_samd21_default_default_XC32_compile_cpp_rule(test_samd21_default_default_XC32_compile_cpp)
    list(APPEND test_samd21_default_library_list "$<TARGET_OBJECTS:test_samd21_default_default_XC32_compile_cpp>")

endif()

# Handle files with suffix [cC], for group default-XC32
if(test_samd21_default_default_XC32_FILE_TYPE_dependentObject)
add_library(test_samd21_default_default_XC32_dependentObject OBJECT ${test_samd21_default_default_XC32_FILE_TYPE_dependentObject})
    test_samd21_default_default_XC32_dependentObject_rule(test_samd21_default_default_XC32_dependentObject)
    list(APPEND test_samd21_default_library_list "$<TARGET_OBJECTS:test_samd21_default_default_XC32_dependentObject>")

endif()

# Handle files with suffix elf, for group default-XC32
if(test_samd21_default_default_XC32_FILE_TYPE_bin2hex)
add_library(test_samd21_default_default_XC32_bin2hex OBJECT ${test_samd21_default_default_XC32_FILE_TYPE_bin2hex})
    test_samd21_default_default_XC32_bin2hex_rule(test_samd21_default_default_XC32_bin2hex)
    list(APPEND test_samd21_default_library_list "$<TARGET_OBJECTS:test_samd21_default_default_XC32_bin2hex>")

endif()


# Main target for this project
add_executable(test_samd21_default_image_5irDnAg8 ${test_samd21_default_library_list})

set_target_properties(test_samd21_default_image_5irDnAg8 PROPERTIES
    OUTPUT_NAME "default"
    SUFFIX ".elf"
    RUNTIME_OUTPUT_DIRECTORY "${test_samd21_default_output_dir}")
target_link_libraries(test_samd21_default_image_5irDnAg8 PRIVATE ${test_samd21_default_default_XC32_FILE_TYPE_link})

# Add the link options from the rule file.
test_samd21_default_link_rule( test_samd21_default_image_5irDnAg8)

# Call bin2hex function from the rule file
test_samd21_default_bin2hex_rule(test_samd21_default_image_5irDnAg8)
add_custom_target(
    merge_loadable_files ALL
    COMMAND hexmate  c:/Users/insoo/Desktop/touch_ml_porting_fin/firmware/test_samd21.X/dist/default/production/test_samd21.X.production.hex ${CMAKE_CURRENT_SOURCE_DIR}/../../../out/test_samd21/default.hex  -O${CMAKE_CURRENT_SOURCE_DIR}/../../../out/test_samd21/default-unified.hex
    BYPRODUCTS ${CMAKE_CURRENT_SOURCE_DIR}/../../../out/test_samd21/default-unified.hex
    COMMENT "Merging loadable hex files into unified image")
add_dependencies(merge_loadable_files test_samd21_default_Bin2Hex)

