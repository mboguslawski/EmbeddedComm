add_library(picoSlaveUSB STATIC
	../../GenericSlave.cpp
	picoSlaveUSB.cpp
)

target_include_directories(picoSlaveUSB PUBLIC
	${CMAKE_CURRENT_LIST_DIR}
	../../

)

target_link_libraries(picoSlaveUSB PUBLIC
	pico_stdlib 
    tinyusb_device
    tinyusb_board
)
