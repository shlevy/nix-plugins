cmake_minimum_required (VERSION 3.9)
project (nix-plugins)
set (nix-plugins_VERSION_MAJOR 3)
set (nix-plugins_VERSION_MINOR 0)

find_package(PkgConfig)

pkg_check_modules(NIX REQUIRED nix-expr>=2.0)

add_library(nix-extra-builtins MODULE extra-builtins.cc)
target_include_directories(nix-extra-builtins PUBLIC ${NIX_INCLUDE_DIRS})
target_compile_options(nix-extra-builtins PUBLIC ${NIX_CFLAGS_OTHER})
