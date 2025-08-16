add_library(rpxxxx_i2c_instrument STATIC
    I2CInstrument.c
)

target_include_directories(rpxxxx_i2c_instrument PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(rpxxxx_i2c_instrument PUBLIC
    pico_stdlib
    hardware_i2c
	hardware_gpio
	pico_i2c_slave
)
