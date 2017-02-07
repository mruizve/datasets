cmake_minimum_required( VERSION 2.8 )
project( image-datasets )

# Eigen library
include_directories( "/usr/include/eigen3/" )

# OpenCV
find_package( OpenCV 3.1 REQUIRED )

# MATLAB
set( MATLAB_ROOT_DIR "/usr/local/MATLAB/R2016a/extern/" )
include_directories( "${MATLAB_ROOT_DIR}/include" )
link_directories( "${MATLAB_ROOT_DIR}/bin/glnxa64" )

# project directories structure
include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/include" )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin" )
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lib" )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lib" )

add_executable( aligner src/aligner.cpp )
target_link_libraries( aligner ${OpenCV_LIBS} )
#target_compile_definitions( aligner PRIVATE DEBUGGING="" )

file(GLOB Conversion_SRC src/convert/*.cpp)
add_executable( converter ${Conversion_SRC} src/converter.cpp )
target_link_libraries( converter ${OpenCV_LIBS} )
#target_compile_definitions( converter PRIVATE DEBUGGING="" )
