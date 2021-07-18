cmake_minimum_required(VERSION 3.0)


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

include_directories( ${BASE_PATH}/src)
include_directories( ${BASE_PATH}/src/tasks/task_usb)
include_directories( ${BASE_PATH}/src/tasks/task_usb/LUFA)
set(SRC_PATH     "${BASE_PATH}/src")

# Compiler flags
set(CMCU      "-mmcu=${MCU}")

set(CFLAGS   "${CMCU} -Wall -Wstrict-prototypes -std=gnu99")
set(CXXFLAGS "${CMCU} -Wall")

set(CMAKE_C_FLAGS   "${CFLAGS}")
set(CMAKE_CXX_FLAGS "${CXXFLAGS}")
set(CMAKE_ASM_FLAGS   "${CFLAGS}")

add_compile_options(-Os) # optimize for file size
add_compile_options(-fdata-sections)
add_compile_options(-ffunction-sections)
add_compile_options(-fpack-struct)
add_compile_options(-fshort-enums)
add_compile_options(-funsigned-char)
add_compile_options(-funsigned-bitfields)
add_compile_options(-fno-exceptions)

add_link_options(LINKER:-Map,skydrop.map,--cref)
add_link_options(-mrelax)
add_link_options(LINKER:--gc-sections)
add_link_options(LINKER:-u,vfprintf)
add_link_options(LINKER:--section-start=.fw_info=0x00810000)
add_link_options(LINKER:--section-start=.eeprom=0x00810020)
add_link_options(LINKER:--section-start=.cfg_ro=0x00810680)

link_libraries(printf_flt)
link_libraries(m)



add_subdirectory("src")

# Print all files
foreach(STR ${SRC_FILES})
    message("file: ${STR}")
endforeach()

# Project setup
add_executable(${PROJECT_NAME} ${SRC_FILES})
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "${PROJECT_NAME}.elf")


add_custom_target(strip  ALL    ${AVRSTRIP} "${PROJECT_NAME}.elf" -o "${PROJECT_NAME}_min.elf" DEPENDS ${PROJECT_NAME})
add_custom_target(hex    ALL    ${OBJCOPY} -R .eeprom -O ihex -R .fw_info -R .cfg_ro "${PROJECT_NAME}.elf" "${PROJECT_NAME}.hex" DEPENDS strip)
add_custom_target(eeprom ALL    ${OBJCOPY} -j .eeprom --change-section-lma .eeprom=0x20 -O ihex "${PROJECT_NAME}.elf" "${PROJECT_NAME}.eep" DEPENDS strip)
add_custom_target(lss    ALL    ${OBJDUMP} -h -S "${PROJECT_NAME}.elf" > "${PROJECT_NAME}.lss" DEPENDS strip)

add_custom_target(binary ALL  "${CMAKE_CURRENT_SOURCE_DIR}/utils/build/post_build.sh" DEPENDS hex eeprom lss)


#set(PROG_TYPE usbasp)
#add_custom_target(flash ${AVRDUDE} -c ${PROG_TYPE} -p ${MCU} -U flash:w:${PROJECT_NAME}.hex DEPENDS hex)


set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${PROJECT_NAME}_min.elf;${PROJECT_NAME}.hex;${PROJECT_NAME}.eep;${PROJECT_NAME}.lss;UPDATE.EE;UPDATE.FW;SKYDROP.FW")

