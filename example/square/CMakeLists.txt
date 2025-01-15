cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Comment out these lines for the RP2040
set(PICO_BOARD pico2)
set(PICO_PLATFORM rp2350)

# Default path to the SDK
if (NOT PICO_SDK_PATH)
  set(PICO_SDK_PATH "../pico-sdk")
endif ()

# Pull in Pico SDK (must be before project)
include(${PICO_SDK_PATH}/pico_sdk_init.cmake)

project(square_test C CXX ASM)

pico_sdk_init()

add_executable(square_test main.c ../../src/mipi_driver.c)

target_include_directories(square_test INTERFACE ../../src/include)

pico_set_program_name(square_test "square test")
pico_set_program_version(square_test "0.1")

pico_enable_stdio_usb(square_test 1)
pico_enable_stdio_uart(square_test 0)

target_link_libraries(square_test pico_stdlib)

target_link_libraries(square_test
        hardware_spi
        hardware_dma
		pico_float
		pico_double

)

#pico_set_float_implementation(hagl_test pico)
#pico_set_double_implementation(hagl_test pico)

pico_add_extra_outputs(square_test)

add_compile_definitions(square_test PRIVATE
	
	DEBUG_MIPI=1
	MIPI_USE_DMA=1

)
