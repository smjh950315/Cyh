cmake_minimum_required(VERSION 3.22)
project(cyhcore)

set(GCC_COVERAGE_COMPILE_FLAGS "-fpermissive")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

# add include dir
include_directories(
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

#link_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_SHARED_LINKER_FLAGS
    "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-rpath -Wl,$ORIGIN -Wl,-fPIC")

# add cpp files
list(APPEND CYHCORE_SRCS
"simdutf.cpp"
"cyh/container/matrix.cpp"
"cyh/container/ndarray.cpp"
"cyh/graphic/image.cpp"
"cyh/graphic/stb_callback.cpp"
"cyh/interop/c_object.cpp"
"cyh/interop/dl_library.cpp"
"cyh/time/datetime.cpp"
"cyh/time/time_details.cpp"
"cyh/time/timer.cpp"
"cyh/time/timespan.cpp"
"cyh/buffer.cpp"
"cyh/console.cpp"
"cyh/exceptions.cpp"
"cyh/filesys.cpp"
"cyh/memory_helper.cpp"
"cyh/memory_stream.cpp"
"cyh/numeric.cpp"
"cyh/task.cpp"
"cyh/text.cpp"
)

add_library(cyhcore SHARED ${CYHCORE_SRCS})
set_target_properties(cyhcore PROPERTIES OUTPUT_NAME cyhcore)
target_compile_features(
    cyhcore
    PUBLIC
    cxx_std_20
)
#target_link_libraries(
#    cyhcore
#    PUBLIC
#    z
#    png
#)

add_library(cyhcore_static STATIC ${CYHCORE_SRCS})
set_target_properties(cyhcore_static PROPERTIES OUTPUT_NAME cyhcores)
target_compile_features(
    cyhcore_static
    PUBLIC
    cxx_std_20
)
#target_link_libraries(
#    cyhcore_static
#    PUBLIC
#    z
#    png
#)
