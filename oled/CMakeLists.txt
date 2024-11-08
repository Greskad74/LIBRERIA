cmake_minimum_required(VERSION 3.13)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)

project(oled)

set(CMAKE_C_STANDARD 11)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# rest of your project

add_executable(oled
	oled.c ../dandan.c
)

target_include_directories(oled
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/../
)

target_link_libraries(oled pico_stdlib hardware_i2c)

pico_enable_stdio_usb(oled 1) 
pico_enable_stdio_uart(oled 0) 

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(oled)

