cmake_minimum_required(VERSION 3.18)
project(yatcpu-debug C CXX ASM)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0")
include_directories(${CMAKE_SOURCE_DIR}/include)
add_definitions(-DLITENES_DEBUG)
add_library(fce
	${CMAKE_SOURCE_DIR}/src/fce/common.c
	${CMAKE_SOURCE_DIR}/src/fce/cpu-addressing.c
	${CMAKE_SOURCE_DIR}/src/fce/cpu.c
	${CMAKE_SOURCE_DIR}/src/fce/fce.c
	${CMAKE_SOURCE_DIR}/src/fce/memory.c
	${CMAKE_SOURCE_DIR}/src/fce/mmc.c
	${CMAKE_SOURCE_DIR}/src/fce/ppu.c
	${CMAKE_SOURCE_DIR}/src/fce/psg.c
)
add_executable(litenes 
	${CMAKE_SOURCE_DIR}/src/main.c
  ${CMAKE_SOURCE_DIR}/src/hal.c
  ${CMAKE_SOURCE_DIR}/src/rom.c
)
target_link_libraries(litenes fce)