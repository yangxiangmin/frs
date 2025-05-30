#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "AWS::aws-crt-cpp" for configuration "Release"
set_property(TARGET AWS::aws-crt-cpp APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(AWS::aws-crt-cpp PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib64/libaws-crt-cpp.so"
  IMPORTED_SONAME_RELEASE "libaws-crt-cpp.so"
  )

list(APPEND _IMPORT_CHECK_TARGETS AWS::aws-crt-cpp )
list(APPEND _IMPORT_CHECK_FILES_FOR_AWS::aws-crt-cpp "${_IMPORT_PREFIX}/lib64/libaws-crt-cpp.so" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
