# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles/RapidWebForgeApp_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/RapidWebForgeApp_autogen.dir/ParseCache.txt"
  "RapidWebForgeApp_autogen"
  )
endif()
