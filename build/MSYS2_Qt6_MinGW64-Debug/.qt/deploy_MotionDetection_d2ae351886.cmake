include("C:/Users/victus/Documents/MotionDetection/build/MSYS2_Qt6_MinGW64-Debug/.qt/QtDeploySupport.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/MotionDetection-plugins.cmake" OPTIONAL)
set(__QT_DEPLOY_I18N_CATALOGS "qtbase")

qt6_deploy_runtime_dependencies(
    EXECUTABLE C:/Users/victus/Documents/MotionDetection/build/MSYS2_Qt6_MinGW64-Debug/MotionDetection.exe
    GENERATE_QT_CONF
)
