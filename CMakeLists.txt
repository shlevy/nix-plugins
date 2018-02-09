cmake_minimum_required (VERSION 3.9)
project (nix-plugins)
set (nix-plugins_VERSION_MAJOR 3)
set (nix-plugins_VERSION_MINOR 0)

find_package(PkgConfig)

pkg_check_modules(NIX REQUIRED nix-expr>=2.0 nix-main>=2.0 nix-store>=2.0)

add_library(nix-extra-builtins MODULE extra-builtins.cc)
configure_file(nix-plugins-config.h.in nix-plugins-config.h)
target_include_directories(nix-extra-builtins PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(nix-extra-builtins PUBLIC ${NIX_INCLUDE_DIRS})
target_compile_options(nix-extra-builtins PUBLIC ${NIX_CFLAGS_OTHER})
