cmake_minimum_required(VERSION 3.12)
project(isl_practice C)

include(ExternalProject)

set(ISL_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/isl_install)
set(ISL_INCLUDE_DIR ${ISL_INSTALL_DIR}/include)
set(ISL_LIBRARY_DIR ${ISL_INSTALL_DIR}/lib)

ExternalProject_Add(isl_external
    SOURCE_DIR        ${CMAKE_CURRENT_SOURCE_DIR}/isl
    PREFIX            ${ISL_INSTALL_DIR}
    CONFIGURE_COMMAND cd <SOURCE_DIR> && ./autogen.sh && ./configure --prefix=<INSTALL_DIR> --with-gmp=system
    BUILD_COMMAND     cd <SOURCE_DIR> && $(MAKE)
    INSTALL_COMMAND   cd <SOURCE_DIR> && $(MAKE) install
    BUILD_BYPRODUCTS  ${ISL_LIBRARY_DIR}/libisl.a
)

add_executable(tiling tiling.c)
add_dependencies(tiling isl_external)
target_include_directories(tiling PRIVATE ${ISL_INCLUDE_DIR})
target_link_libraries(tiling PRIVATE
    ${ISL_LIBRARY_DIR}/libisl.a
    gmp
)

add_executable(tiling_2d tiling_2d.c)
add_dependencies(tiling_2d isl_external)
target_include_directories(tiling_2d PRIVATE ${ISL_INCLUDE_DIR})
target_link_libraries(tiling_2d PRIVATE
    ${ISL_LIBRARY_DIR}/libisl.a
    gmp
)

add_executable(generate_fixed_loop generate_fixed_loop.c)
add_dependencies(generate_fixed_loop isl_external)
target_include_directories(generate_fixed_loop PRIVATE ${ISL_INCLUDE_DIR})
target_link_libraries(generate_fixed_loop PRIVATE
    ${ISL_LIBRARY_DIR}/libisl.a
    gmp
)
