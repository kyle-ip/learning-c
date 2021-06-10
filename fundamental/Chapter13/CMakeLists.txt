cmake_minimum_required(VERSION 3.10.2)

get_filename_component(ProjectId ${CMAKE_CURRENT_SOURCE_DIR} NAME)
string(REPLACE " " "_" ProjectId ${ProjectId})
project(${ProjectId} C)

set(CMAKE_C_STANDARD 11)

set(CMAKE_INSTALL_PREFIX install)

if(APPLE)
    set(CMAKE_INSTALL_RPATH @loader_path/../lib)
elseif (UNIX)
    set(CMAKE_INSTALL_RPATH \$ORIGIN/../lib)
endif ()

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

include_directories("include")

add_subdirectory(fibonacci)
add_subdirectory(factorial)

file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
foreach(file ${files})
    get_filename_component(name ${file} NAME)
    add_executable(${name} ${file})

    target_link_libraries(${name}
            factorial
            fibonacci
            ${CONAN_LIBS}
            )

    install(TARGETS ${name} RUNTIME DESTINATION bin)
endforeach()