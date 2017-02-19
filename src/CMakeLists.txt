file( GLOB LibIO_SRC libio/*.cpp )
add_library( io SHARED ${LibIO_SRC} )
target_link_libraries( io ${OpenCV_LIBS} ${MATLAB_LIBS} )
#target_compile_definitions( io PRIVATE DEBUGGING="" )

add_executable( aligner aligner.cpp )
target_link_libraries( aligner ${OpenCV_LIBS} )
#target_compile_definitions( aligner PRIVATE DEBUGGING="" )

add_executable( converter converter.cpp )
target_link_libraries( converter io )
#target_compile_definitions( converter PRIVATE DEBUGGING="" )

add_executable( visualizer visualizer.cpp )
target_link_libraries( visualizer io )
#target_compile_definitions( visualizer PRIVATE DEBUGGING="" )

#file( GLOB Converter_Old_SRC converter-old/*.cpp )
#add_executable( converter-old ${Converter_Old_SRC} )
#target_link_libraries( converter-old ${OpenCV_LIBS} ${MATLAB_LIBS} )
#target_compile_definitions( converter-old PRIVATE DEBUGGING="" )

file( GLOB DMatrix_CXX dmatrix/*.cpp )
file( GLOB DMatrix_CU dmatrix/*.cu )
set( DMatrix_SRC "${DMatrix_CXX}" "${DMatrix_CU}" )
cuda_add_executable( dmatrix ${DMatrix_SRC} )
target_link_libraries( dmatrix io ${OpenCV_LIBS} )
#target_compile_definitions( dmatrix PRIVATE DEBUGGING="" )
