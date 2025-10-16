include("D:/Proj_cpp/untitled/build/Desktop_Qt_6_9_2_MinGW_64_bit-MinSizeRel/.qt/QtDeploySupport.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/untitled-plugins.cmake" OPTIONAL)
set(__QT_DEPLOY_I18N_CATALOGS "qtbase")

qt6_deploy_runtime_dependencies(
    EXECUTABLE "D:/Proj_cpp/untitled/build/Desktop_Qt_6_9_2_MinGW_64_bit-MinSizeRel/untitled.exe"
    GENERATE_QT_CONF
)
