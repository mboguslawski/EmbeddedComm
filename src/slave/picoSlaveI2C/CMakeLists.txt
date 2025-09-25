add_library(picoSlaveI2C STATIC
	../GenericSlave/GenericSlave.cpp
	picoSlaveI2C.cpp
)

target_include_directories(picoSlaveI2C PUBLIC
	${CMAKE_CURRENT_LIST_DIR}
	../GenericSlave/

)

target_link_libraries(picoSlaveI2C PUBLIC
	pico_stdlib
	hardware_i2c
	hardware_gpio
	pico_i2c_slave
)
