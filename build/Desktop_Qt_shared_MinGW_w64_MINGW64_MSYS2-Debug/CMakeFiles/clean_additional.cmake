# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\ar_31_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\ar_31_autogen.dir\\ParseCache.txt"
  "ar_31_autogen"
  )
endif()
