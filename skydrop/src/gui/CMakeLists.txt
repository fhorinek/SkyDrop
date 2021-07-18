set(DIR ${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB SRC_SETTINGS "${DIR}/settings/*.c*")
file(GLOB SRC_WIDGETS "${DIR}/widgets/layouts/*.c*")
file(GLOB SRC_WIDGETS2 "${DIR}/widgets/*.c*")
file(GLOB SRC_GUI "${DIR}/gui_*.c*")



set(SRC_FILES ${SRC_FILES} 
${SRC_WIDGETS}
${SRC_WIDGETS2}
${SRC_SETTINGS}
${SRC_GUI}
"${DIR}/pages.cpp"
"${DIR}/splash.cpp"
"${DIR}/update.cpp"
"${DIR}/usb.cpp"
"${DIR}/gui.cpp"
PARENT_SCOPE)