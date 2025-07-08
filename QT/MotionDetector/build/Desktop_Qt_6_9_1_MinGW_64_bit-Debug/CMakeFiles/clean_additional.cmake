# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\MotionDetectProject_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\MotionDetectProject_autogen.dir\\ParseCache.txt"
  "MotionDetectProject_autogen"
  )
endif()
