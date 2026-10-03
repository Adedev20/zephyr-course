# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(hello_test)

target_sources(app PRIVATE src/main.c)

# Pull in your driver source subdirectory
add_subdirectory(drivers/our_led_sensor)

