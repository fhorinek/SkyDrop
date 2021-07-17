cmake_minimum_required(VERSION 3.0)

set(PROG_TYPE usbasp)


# Variables regarding the AVR chip
set(MCU   atxmega192a3u)
set(F_CPU 32000000)
set(BAUD  9600)
add_definitions(-DF_CPU=${F_CPU})
add_definitions(-DUSE_LUFA_CONFIG_HEADER)
add_definitions(-DF_USB=48000000)
add_definitions(-DARCH=ARCH_XMEGA)
add_definitions(-DBOARD=BOARD_USER)

# program names
set(VERBOSE true)


set(AVR "${CMAKE_CURRENT_SOURCE_DIR}/avr/bin/")
set(AVRCPP   "${AVR}avr-g++")
set(AVRC     "${AVR}avr-gcc")
set(AVRSTRIP "${AVR}avr-strip")
set(OBJCOPY  "${AVR}avr-objcopy")
set(OBJDUMP  "${AVR}avr-objdump")
set(AVRSIZE  "${AVR}avr-size")
set(AVRDUDE  "${AVR}avrdude")

# Sets the compiler
# Needs to come before the project function
set(CMAKE_SYSTEM_NAME  Generic)
set(CMAKE_CXX_COMPILER ${AVRCPP})
set(CMAKE_C_COMPILER   ${AVRC})
set(CMAKE_ASM_COMPILER   ${AVRC})

project (skydrop C CXX ASM)

# Important project paths
set(BASE_PATH    "${${PROJECT_NAME}_SOURCE_DIR}")

option(IGC "use igc private key" ON)
if(NOT ${IGC})
    add_definitions(-DIGC_NO_PRIVATE_KEY)
endif()


include_directories( ${BASE_PATH}/src)
include_directories( ${BASE_PATH}/src/tasks/task_usb)
include_directories( ${BASE_PATH}/src/tasks/task_usb/LUFA)

set(SRC_PATH     "${BASE_PATH}/src")

# Files to be compiled
#set(SRC_FILES "${SRC_PATH}/skydrop.cpp")


# Compiler flags
set(CSTANDARD "-std=gnu99")
#set(CDEBUG    "-gstabs -g -ggdb")
set(CDEBUG "")
#set(CWARN     "-Wall -Wstrict-prototypes")
#set(CTUNING   "-funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums -ffunction-sections -fdata-sections")
set(CTUNING "-fdata-sections")
set(CTUNING "${CTUNING}  -ffunction-sections")
#set(CTUNING " ${CTUNING}  -mtiny-stack -fno-tree-ter -fno-caller-saves -mstrict-X -fno-optimize-sibling-calls -mcall-prologues -finline-limit=5 -fno-split-wide-types -fno-tree-scev-cprop")
#set(CTUNING "${CTUNING} -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums ")
set(COPT      "-Os")
set(LOPT  "-Wl,-u,vfprintf -lm -Wl,--section-start=.fw_info=0x00810000 -Wl,--section-start=.eeprom=0x00810020 -Wl,--section-start=.cfg_ro=0x00810680")
#set(LOPT "${LOPT} -Wl,-nostdlib")
#set(LOPT "${LOPT} -mrelax")
#set(LOPT "${LOPT} -Wl,-s")
set(LOPT "${LOPT} -Wl,--gc-sections")

set(CMCU      "-mmcu=${MCU}")
#set(CDEFS     "-DF_CPU=${F_CPU}") # -DBAUD=${BAUD}

set(CFLAGS   "${CMCU} ${CDEBUG} ${CDEFS} ${COPT} ${LOPT} ${CWARN} ${CSTANDARD} ${CTUNING}")
set(CXXFLAGS "${CMCU} ${CDEBUG} ${CDEFS} ${COPT} ${LOPT} ${CTUNING}")

set(CMAKE_C_FLAGS   "${CFLAGS}")
set(CMAKE_CXX_FLAGS "${CXXFLAGS}")
set(CMAKE_ASM_FLAGS   "${CFLAGS}")

#add_link_options(-mrelax )

#set(SRC_FILES)
#set(HEADER_FILES)
add_subdirectory("src")


message("headers")

foreach(STR ${SRC_FILES})
    #message("file: ${STR}")
endforeach()
# Project setup
add_executable(${PROJECT_NAME} ${SRC_FILES})
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "${PROJECT_NAME}.elf")

#target_link_libraries(${PROJECT_NAME} printf_flt)
#target_link_libraries(${PROJECT_NAME} m)
# Compiling targets

add_custom_target(strip  ALL    ${AVRSTRIP} "${PROJECT_NAME}.elf" -o "${PROJECT_NAME}_min.elf" DEPENDS ${PROJECT_NAME})
add_custom_target(hex    ALL    ${OBJCOPY} -R .eeprom -O ihex -R .fw_info -R .cfg_ro "${PROJECT_NAME}.elf" "${PROJECT_NAME}.hex" DEPENDS strip)
add_custom_target(eeprom ALL    ${OBJCOPY} -j .eeprom --change-section-lma .eeprom=0x20 -O ihex "${PROJECT_NAME}.elf" "${PROJECT_NAME}.eep" DEPENDS strip)
add_custom_target(lss    ALL    ${OBJDUMP} -h -S "${PROJECT_NAME}.elf" > "${PROJECT_NAME}.lss" DEPENDS strip)

add_custom_target(binary ALL  "${CMAKE_CURRENT_SOURCE_DIR}/utils/build/post_build.sh" DEPENDS hex eeprom lss)

#add_custom_target(flash ${AVRDUDE} -c ${PROG_TYPE} -p ${MCU} -U flash:w:${PROJECT_NAME}.hex DEPENDS hex)


set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${PROJECT_NAME}.hex;${PROJECT_NAME}.eeprom;${PROJECT_NAME}.lst")

