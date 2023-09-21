cmake_minimum_required(VERSION 3.5)
project(forwardsdk)
set(gcpplis "")
set(gincdir "")

function(add_cpp cppdir)
    set(cpplistmp)
    list(APPEND cpplistmp ${gcpplis})
    list(APPEND cpplistmp ${cppdir}/*.cpp)
    list(APPEND cpplistmp ${cppdir}/*.hpp)
    list(APPEND cpplistmp ${cppdir}/*.c)
    list(APPEND cpplistmp ${cppdir}/*.cxx)
    file(GLOB_RECURSE cpplistmp ${cpplistmp})

    set(gcpplis "${cpplistmp}" PARENT_SCOPE)
endfunction()


function(add_inc incdir)
    set(incdirtmp)
    list(APPEND incdirtmp ${gincdir})
    list(APPEND incdirtmp ${incdir})
    set(gincdir "${incdir}" PARENT_SCOPE)
endfunction()

add_cpp(${CMAKE_SOURCE_DIR}/src/source)
add_inc(${CMAKE_SOURCE_DIR}/src/include)
# message(${gcpplis})

include_directories(
    ${gincdir}
)

add_library(sdk SHARED ${gcpplis})

target_compile_options(
    sdk
    PUBLIC
    -std=c++11
    -O3
    -g
    -Werror=return-type
    -pthread
)

file(GLOB_RECURSE DEMO example/*.cpp)
file(GLOB_RECURSE DEMO1 example/*.cxx)
set(DEMO "${DEMO};${DEMO1}")
message("rizhi ${DEMO}")

find_package(Threads)

foreach(filee ${DEMO})
    get_filename_component(targetname ${filee} NAME_WE)
    message(${filee})
    add_executable(${targetname} ${filee})
    target_link_libraries(${targetname} PUBLIC sdk Threads::Threads)
    target_include_directories(${targetname} PUBLIC ${gincdir})
    target_compile_options(
        ${targetname}
        PUBLIC
        -std=c++11
        -O3
        -g
        -fopenmp
    )
endforeach(filee)
