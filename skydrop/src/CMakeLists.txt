set(SRC_FILES)

#file(GLOB SRC_FILES_TMP "${SRC_PATH}/xlib/*.c*")
#set(SRC_FILES ${SRC_FILES} ${SRC_FILES_TMP})
set(SRC_FILES ${SRC_FILES}
"${SRC_PATH}/xlib/common.c"
"${SRC_PATH}/xlib/core/adc.cpp"
"${SRC_PATH}/xlib/core/clock.cpp"
"${SRC_PATH}/xlib/core/dac.cpp"
"${SRC_PATH}/xlib/core/gpio.cpp"
"${SRC_PATH}/xlib/core/i2c.cpp"
"${SRC_PATH}/xlib/core/rtc.cpp"
"${SRC_PATH}/xlib/core/spi.cpp"
"${SRC_PATH}/xlib/core/system.cpp"
"${SRC_PATH}/xlib/core/timer.cpp"
"${SRC_PATH}/xlib/core/usart.cpp"
"${SRC_PATH}/xlib/core/watchdog.cpp"
"${SRC_PATH}/xlib/ring_small.cpp"
"${SRC_PATH}/xlib/ring.cpp"
)


set(SRC_FILES ${SRC_FILES} "${SRC_PATH}/fc/conf.cpp")
add_subdirectory("${SRC_PATH}/fc")


file(GLOB SRC_FILES_TMP "${SRC_PATH}/data/*.c*")
set(SRC_FILES ${SRC_FILES} ${SRC_FILES_TMP})

file(GLOB SRC_FILES_TMP "${SRC_PATH}/drivers/audio/*.c*")
set(SRC_FILES ${SRC_FILES} ${SRC_FILES_TMP})

file(GLOB SRC_FILES_TMP "${SRC_PATH}/drivers/bluetooth/*.c*")
set(SRC_FILES ${SRC_FILES} ${SRC_FILES_TMP})

file(GLOB SRC_FILES_TMP "${SRC_PATH}/drivers/sensors/*.c*")
set(SRC_FILES ${SRC_FILES} ${SRC_FILES_TMP})

file(GLOB SRC_FILES_TMP "${SRC_PATH}/drivers/storage/FatFs/*.c*")
set(SRC_FILES ${SRC_FILES} ${SRC_FILES_TMP})

file(GLOB SRC_FILES_TMP "${SRC_PATH}/drivers/storage/*.c*")
set(SRC_FILES ${SRC_FILES} ${SRC_FILES_TMP})

file(GLOB SRC_FILES_TMP "${SRC_PATH}/drivers/*.c*")
set(SRC_FILES ${SRC_FILES} ${SRC_FILES_TMP})


#file(GLOB SRC_FILES_TMP "${SRC_PATH}/fc/*.c*")
#set(SRC_FILES ${SRC_FILES} ${SRC_FILES_TMP})
set(UDIR "${SRC_PATH}/tasks/task_usb/LUFA/Drivers/USB/Core/XMEGA")

set(SRC_FILES ${SRC_FILES} 
"${UDIR}/Endpoint_XMEGA.c" 
"${UDIR}/EndpointStream_XMEGA.c" 
"${UDIR}/Device_XMEGA.c"
"${UDIR}/USBController_XMEGA.c"
"${UDIR}/USBInterrupt_XMEGA.c"


)

#file(GLOB_RECURSE SRC_FILES_TMP "${SRC_PATH}/tasks/task_usb/LUFA/Drivers/USB/Core/XMEGA/Device_XMEGA.c*")
#set(SRC_FILES ${SRC_FILES} ${SRC_FILES_TMP})

file(GLOB SRC_FILES_TMP "${SRC_PATH}/tasks/task_usb/LUFA/Drivers/USB/Core/*.c*")
set(SRC_FILES ${SRC_FILES} ${SRC_FILES_TMP})

#set(SRC_FILES ${SRC_FILES} "${SRC_PATH}/tasks/task_usb/LUFA/Drivers/USB/Core/XMEGA/Device_XMEGA.c")
file(GLOB SRC_FILES_TMP "${SRC_PATH}/tasks/task_usb/MassStorage/Config/*.c*")
set(SRC_FILES ${SRC_FILES} ${SRC_FILES_TMP})
file(GLOB SRC_FILES_TMP "${SRC_PATH}/tasks/task_usb/MassStorage/Lib/*.c*")
set(SRC_FILES ${SRC_FILES} ${SRC_FILES_TMP})
file(GLOB SRC_FILES_TMP "${SRC_PATH}/tasks/task_usb/MassStorage/*.c*")
set(SRC_FILES ${SRC_FILES} ${SRC_FILES_TMP})
file(GLOB SRC_FILES_TMP "${SRC_PATH}/tasks/task_usb/task_usb.c*")

set(SRC_FILES ${SRC_FILES} ${SRC_FILES_TMP})
file(GLOB SRC_FILES_TMP "${SRC_PATH}/tasks/*.c*")
set(SRC_FILES ${SRC_FILES} ${SRC_FILES_TMP})


add_subdirectory("${SRC_PATH}/gui")

file(GLOB SRC_FILES_TMP "${SRC_PATH}/*.c*")
set(SRC_FILES ${SRC_FILES} ${SRC_FILES_TMP})

set(SRC_FILES ${SRC_MODULES} ${SRC_FILES} PARENT_SCOPE) 