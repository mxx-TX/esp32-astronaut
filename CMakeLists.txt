# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.22)

set(EXTRA_COMPONENT_DIRS ${CMAKE_CURRENT_LIST_DIR}/bsp
                         ${CMAKE_CURRENT_LIST_DIR}/os
                         ${CMAKE_CURRENT_LIST_DIR}/svc
                         ${CMAKE_CURRENT_LIST_DIR}/app
                         ${CMAKE_CURRENT_LIST_DIR}/drv)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(esp32s3-astronaut)
