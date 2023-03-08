cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(phrader C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17) 
pico_sdk_init()
 
add_compile_definitions(NUM_CHANNELS=8
LED_PIN_OLD=0 #set this to 1 if the circuit board is before rev 3
FW_VERSION_NUMBER=2)
 
add_executable(phrader
  phrader.c
  dac.c
  potentiometer.c 
  leds_driver.c
  leds.c
  buttons.c
  internal_timer.c
)

target_include_directories(phrader PRIVATE ${CMAKE_CURRENT_LIST_DIR})

pico_add_extra_outputs(phrader)

target_link_libraries(phrader PRIVATE 
pico_stdlib 
hardware_pwm
hardware_adc 
hardware_pio
hardware_clocks
hardware_dma
)

pico_enable_stdio_usb(phrader 1)

pico_generate_pio_header(phrader ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio  OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/generated)
target_sources(phrader PRIVATE phrader.c)