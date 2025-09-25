add_library(picoMasterI2C STATIC
	picoMasterI2C.cpp
)

target_include_directories(picoMasterI2C PUBLIC
	${CMAKE_CURRENT_LIST_DIR}
	../I2CConductor/
)

target_link_libraries(picoMasterI2C PUBLIC
	pico_stdlib
	hardware_i2c
	hardware_gpio
)
