set(SRC_FILES)
# We add all the source files here.


# add xlib
file(GLOB SRC_FILES_TMP "${SRC_PATH}/xlib/core/*.cpp")
set(SRC_FILES ${SRC_FILES} ${SRC_FILES_TMP})

set(SRC_FILES ${SRC_FILES}
    "${SRC_PATH}/xlib/common.c"
    "${SRC_PATH}/xlib/ring_small.cpp"
    "${SRC_PATH}/xlib/ring.cpp"
)

# add fc
file(GLOB SRC_FILES_TMP "${SRC_PATH}/fc/*.c*")
set(SRC_FILES ${SRC_FILES} ${SRC_FILES_TMP})
file(GLOB SRC_FILES_TMP "${SRC_PATH}/fc/protocols/*.c*")
set(SRC_FILES ${SRC_FILES} ${SRC_FILES_TMP})

set(SRC_FILES ${SRC_FILES}
    "${SRC_PATH}/fc/logger/logger.cpp"
    "${SRC_PATH}/fc/logger/igc.cpp"
    "${SRC_PATH}/fc/logger/kml.cpp"
    "${SRC_PATH}/fc/logger/sha256.cpp"
)


file(GLOB SRC_FILES_TMP "${SRC_PATH}/data/*.c*")
set(SRC_FILES ${SRC_FILES} ${SRC_FILES_TMP})

file(GLOB_RECURSE SRC_DRIVER "${SRC_PATH}/drivers/**.c*")
set(SRC_FILES ${SRC_FILES} 
    ${SRC_DRIVER})


## task_usb

# add these files from LUFA driver:
set(UDIR "${SRC_PATH}/tasks/task_usb/LUFA/Drivers/USB/Core/XMEGA")
set(SRC_FILES ${SRC_FILES} 
"${UDIR}/Endpoint_XMEGA.c" 
"${UDIR}/EndpointStream_XMEGA.c" 
"${UDIR}/Device_XMEGA.c"
"${UDIR}/USBController_XMEGA.c"
"${UDIR}/USBInterrupt_XMEGA.c"
)



file(GLOB SRC_FILES_TMP "${SRC_PATH}/tasks/task_usb/LUFA/Drivers/USB/Core/*.c*")
set(SRC_FILES ${SRC_FILES} ${SRC_FILES_TMP})

# add MassStorage
file(GLOB_RECURSE SRC_STORAGE "${SRC_PATH}/tasks/task_usb/MassStorage/**.c*")
set(SRC_FILES ${SRC_FILES} 
    ${SRC_STORAGE})

file(GLOB SRC_FILES_TMP "${SRC_PATH}/tasks/task_usb/task_usb.c*")

set(SRC_FILES ${SRC_FILES} ${SRC_FILES_TMP})
file(GLOB SRC_FILES_TMP "${SRC_PATH}/tasks/*.c*")
set(SRC_FILES ${SRC_FILES} ${SRC_FILES_TMP})


# add /gui recursively
file(GLOB_RECURSE SRC_GUI "${SRC_PATH}/gui/**.c*")
set(SRC_FILES ${SRC_FILES} 
    ${SRC_GUI})

# add outer dir
file(GLOB SRC_FILES_TMP "${SRC_PATH}/*.c*")
set(SRC_FILES ${SRC_FILES} ${SRC_FILES_TMP})

set(SRC_FILES ${SRC_FILES} PARENT_SCOPE) 