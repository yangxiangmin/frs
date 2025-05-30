#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "aws-cpp-sdk-core" for configuration "Release"
set_property(TARGET aws-cpp-sdk-core APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(aws-cpp-sdk-core PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib64/libaws-cpp-sdk-core.so"
  IMPORTED_SONAME_RELEASE "libaws-cpp-sdk-core.so"
  )

list(APPEND _IMPORT_CHECK_TARGETS aws-cpp-sdk-core )
list(APPEND _IMPORT_CHECK_FILES_FOR_aws-cpp-sdk-core "${_IMPORT_PREFIX}/lib64/libaws-cpp-sdk-core.so" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
