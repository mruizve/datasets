cmake_minimum_required( VERSION 2.8 )
project( image-datasets )

# CUDA
find_package( CUDA )
include( FindCUDA )

# Eigen library
include_directories( "/usr/include/eigen3/" )

# OpenCV
find_package( OpenCV 3.1 REQUIRED )

# MATLAB
set( MATLAB_ROOT_DIR "/usr/local/MATLAB/R2016a/" )
set( MATLAB_LIBS_DIR "${MATLAB_ROOT_DIR}/bin/glnxa64" )
set( MATLAB_LIBS "${MATLAB_LIBS_DIR}/libmx.so" "${MATLAB_LIBS_DIR}/libmat.so" )
include_directories( "${MATLAB_ROOT_DIR}/extern/include" )

# project directories structure
include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/include" )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin" )
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lib" )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lib" )

# load targets
subdirs( src )
