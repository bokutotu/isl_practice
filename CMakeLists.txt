cmake_minimum_required(VERSION 3.12)
project(isl_practice C)

# --- find_package(GMP REQUIRED) を削除 ---

include(ExternalProject)

set(ISL_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/isl_install)
set(ISL_INCLUDE_DIR ${ISL_INSTALL_DIR}/include)
set(ISL_LIBRARY_DIR ${ISL_INSTALL_DIR}/lib)

ExternalProject_Add(isl_external
    SOURCE_DIR        ${CMAKE_CURRENT_SOURCE_DIR}/isl
    PREFIX            ${ISL_INSTALL_DIR}
    # Docker(Linux) 用の configure コマンド
    CONFIGURE_COMMAND cd <SOURCE_DIR> && ./autogen.sh && ./configure --prefix=<INSTALL_DIR> --with-gmp=system
    BUILD_COMMAND     cd <SOURCE_DIR> && $(MAKE)
    INSTALL_COMMAND   cd <SOURCE_DIR> && $(MAKE) install
    BUILD_BYPRODUCTS  ${ISL_LIBRARY_DIR}/libisl.a
)

# --- 実行可能ファイルの定義 (simple_loop と仮定) ---
add_executable(tiling tiling.c)

# --- 依存関係 ---
add_dependencies(tiling isl_external)

# --- インクルードディレクトリ ---
target_include_directories(tiling PRIVATE ${ISL_INCLUDE_DIR})

# --- リンク設定 ---
# GMP::GMP の代わりに、直接 'gmp' ライブラリ (-lgmp) をリンクするように指定
target_link_libraries(tiling PRIVATE
    ${ISL_LIBRARY_DIR}/libisl.a
    gmp # リンカに libgmp を探させる
)
