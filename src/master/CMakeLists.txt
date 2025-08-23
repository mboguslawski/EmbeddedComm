add_library(rpxxxx_i2c_conductor STATIC
    I2CConductor.c
)

target_include_directories(rpxxxx_i2c_conductor PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
    ../common
)

target_link_libraries(rpxxxx_i2c_conductor PUBLIC
    pico_stdlib
    hardware_i2c
	hardware_gpio
)
