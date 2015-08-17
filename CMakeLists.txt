cmake_minimum_required(VERSION 2.8)
project(ncRouting)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

if(${CMAKE_HOST_APPLE})
	include_directories(/sw/include)
	link_directories(/sw/lib)
endif(${CMAKE_HOST_APPLE})

include_directories(/usr/local/share/ghaas/include)
link_directories(/usr/local/share/ghaas/lib)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

FILE(GLOB sources src/*.C)

add_executable(routing ${sources})
if(${CMAKE_HOST_APPLE})
	target_link_libraries(routing CM10 netcdf udunits2 m)
else(${CMAKE_HOST_APPLE})
	target_link_libraries(routing CM10 netcdf udunits2 m -pthread)
endif(${CMAKE_HOST_APPLE})
