set(DIR ${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB SRC_PROTOCOL "${DIR}/protocols/*.c*")


set(SRC_FILES ${SRC_FILES} 
"${DIR}/logger/igc.cpp"
"${DIR}/logger/kml.cpp"
"${DIR}/logger/logger.cpp"
"${DIR}/logger/sha256.cpp"
${SRC_PROTOCOL}
"${DIR}/acc.cpp"
"${DIR}/agl.cpp"
"${DIR}/airspace.cpp"
"${DIR}/alt_calibration.cpp"
"${DIR}/circling.cpp"
"${DIR}/compass.cpp"
"${DIR}/conf.cpp"
"${DIR}/gyro.cpp"
"${DIR}/imu.cpp"
"${DIR}/kalman.cpp"
"${DIR}/mag.cpp"
"${DIR}/navigation.cpp"
"${DIR}/vario.cpp"
"${DIR}/waypoint.cpp"
"${DIR}/wind.cpp"
"${DIR}/fc.cpp"
PARENT_SCOPE)