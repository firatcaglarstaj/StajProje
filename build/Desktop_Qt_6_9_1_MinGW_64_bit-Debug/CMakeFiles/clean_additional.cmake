# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\MotionDetection_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\MotionDetection_autogen.dir\\ParseCache.txt"
  "MotionDetection_autogen"
  )
endif()
