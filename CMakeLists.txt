cmake_minimum_required (VERSION 3.9)
project (nix-plugins)
set (nix-plugins_VERSION_MAJOR 4)
set (nix-plugins_VERSION_MINOR 0)
set (nix-plugins_VERSION_PATCH 4)

find_package(PkgConfig)

pkg_check_modules(NIX REQUIRED nix-expr>=2.1 nix-main>=2.1 nix-store>=2.1)

find_path(BOOST_INCLUDE_DIR boost/format.hpp)
if(BOOST_INCLUDE_DIR STREQUAL "BOOST_INCLUDE_DIR-NOTFOUND")
  message(FATAL_ERROR "Could not find Boost formatting library.")
endif()
include_directories(${BOOST_INCLUDE_DIR})

add_library(nix-extra-builtins SHARED extra-builtins.cc)
configure_file(nix-plugins-config.h.in nix-plugins-config.h)
target_include_directories(nix-extra-builtins PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(nix-extra-builtins PUBLIC ${NIX_INCLUDE_DIRS})
target_compile_options(nix-extra-builtins PUBLIC ${NIX_CFLAGS_OTHER})

link_directories(${NIX_LIBRARY_DIRS})
target_link_libraries(nix-extra-builtins ${NIX_LDFLAGS_OTHER})
target_link_libraries(nix-extra-builtins PUBLIC ${NIX_LIBRARIES})

install(TARGETS nix-extra-builtins DESTINATION lib/nix/plugins)
