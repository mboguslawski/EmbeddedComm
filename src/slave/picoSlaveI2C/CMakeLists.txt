add_library(picoSlaveI2C STATIC
	../I2CInstrument/I2CInstrument.cpp
	picoSlaveI2C.cpp
)

target_include_directories(picoSlaveI2C PUBLIC
	${CMAKE_CURRENT_LIST_DIR}
	../I2CInstrument/

)

target_link_libraries(picoSlaveI2C PUBLIC
	pico_stdlib
	hardware_i2c
	hardware_gpio
	pico_i2c_slave
)
