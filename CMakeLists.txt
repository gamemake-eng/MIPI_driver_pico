add_library(MIPI_driver INTERFACE)

target_sources(MIPI_driver INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/src/mipi_driver.c
)

target_include_directories(MIPI_driver INTERFACE ${CMAKE_CURRENT_LIST_DIR}/src/include)

target_link_libraries(MIPI_driver INTERFACE pico_stdlib hardware_spi hardware_gpio)