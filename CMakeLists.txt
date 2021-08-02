cmake_minimum_required (VERSION 3.9)
project (nix-plugins)
set (nix-plugins_VERSION_MAJOR 7)
set (nix-plugins_VERSION_MINOR 0)
set (nix-plugins_VERSION_PATCH 0)

find_package(PkgConfig)

pkg_check_modules(NIX REQUIRED nix-expr>=2.4 nix-main>=2.4 nix-store>=2.4)

find_path(BOOST_INCLUDE_DIR boost/format.hpp)
if(BOOST_INCLUDE_DIR STREQUAL "BOOST_INCLUDE_DIR-NOTFOUND")
  message(FATAL_ERROR "Could not find Boost formatting library.")
endif()
include_directories(${BOOST_INCLUDE_DIR})

find_package(nlohmann_json)

if(APPLE)
  set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -flat_namespace -undefined suppress")
endif()

add_library(nix-extra-builtins MODULE extra-builtins.cc)
configure_file(nix-plugins-config.h.in nix-plugins-config.h)
target_include_directories(nix-extra-builtins PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(nix-extra-builtins PUBLIC ${NIX_INCLUDE_DIRS})
target_compile_options(nix-extra-builtins PUBLIC ${NIX_CFLAGS_OTHER})
target_link_libraries(nix-extra-builtins nlohmann_json::nlohmann_json)

install(TARGETS nix-extra-builtins DESTINATION lib/nix/plugins)
