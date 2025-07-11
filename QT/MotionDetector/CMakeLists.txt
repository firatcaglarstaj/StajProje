cmake_minimum_required(VERSION 3.19)
project(MotionDetection LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Multimedia MultimediaWidgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Multimedia MultimediaWidgets)
find_package(OpenCV REQUIRED)
find_package(PythonLibs REQUIRED)
include_directories(${PYTHON_INCLUDE_DIRS})
qt_standard_project_setup()

qt_add_executable(MotionDetection
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    pythonScriptModel.cpp
    pythonScriptModel.h
    motiondetector.h
    motiondetector.cpp
)

target_link_libraries(MotionDetection
    PRIVATE
        Qt::Core
        Qt::Widgets
        ${OpenCV_LIBS}
        ${PYTHON_LIBRARIES}
    )
include(GNUInstallDirs)

install(TARGETS MotionDetection
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET MotionDetection
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
