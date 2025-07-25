cmake_minimum_required(VERSION 3.19)
project(MotionDetection LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Network)
find_package(OpenCV REQUIRED)

qt_standard_project_setup()

qt_add_executable(MotionDetection
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    core/VideoController.cpp
    mainwindow.ui
    ai/yolocommunicator.h
    ai/yolocommunicator.cpp
    ai/DetectionData.h
    core/motioncontroller.h
    core/motioncontroller.cpp
    motionwindow.h motionwindow.cpp
    movedetect.cpp
)

target_include_directories(MotionDetection PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(MotionDetection PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
    ${OpenCV_LIBS}
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(MotionDetection PRIVATE DEBUG_MODE)
endif()
