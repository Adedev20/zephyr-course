# Set the minimum CMake version required by Zephyr
cmake_minimum_required(VERSION 3.20.0)

# CRITICAL: Forces the build system to parse your custom Kconfig file layout
set(KCONFIG_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/Kconfig)

# Pull in the core Zephyr build environment and package definitions
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

# Define your project name
project(blinky)

# Link your finalized main source file to the application target executable
target_sources(app PRIVATE src/main.c)
