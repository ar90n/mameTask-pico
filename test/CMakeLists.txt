# Top-level CMakeLists.txt for mameTask-pico tests

cmake_minimum_required(VERSION 3.13)

# Set the project name
project(mameTask_pico_tests C CXX ASM)

# Option to select target platform
option(BUILD_FOR_PICO "Build tests for Pico hardware" OFF)

# Common compiler flags
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Common source files
set(TEST_SOURCES
    test_main.cpp
    test_task.cpp
    test_runner.cpp
)

# Device-specific source files
set(DEVICE_SOURCES
    test_device.cpp
)

if(BUILD_FOR_PICO)
    # Configure for Pico build
    include(pico_sdk_import.cmake)
    pico_sdk_init()
    
    # Add definition for platform detection
    add_definitions(-DPICO_BUILD)
    
    # Create test executable
    add_executable(mameTask_tests ${TEST_SOURCES} ${DEVICE_SOURCES})
    
    # Add include directories
    target_include_directories(mameTask_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${CMAKE_CURRENT_SOURCE_DIR}
    )
    
    # Link against Pico SDK
    target_link_libraries(mameTask_tests pico_stdlib)
    
    # Enable USB output, disable UART output
    pico_enable_stdio_usb(mameTask_tests 1)
    pico_enable_stdio_uart(mameTask_tests 0)
    
    # Create map/bin/hex/uf2 files
    pico_add_extra_outputs(mameTask_tests)
else()
    # Configure for host build
    add_executable(mameTask_tests ${TEST_SOURCES})
    
    # Add include directories
    target_include_directories(mameTask_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/mock
    )
    
    # Add any host-specific libraries
    if(UNIX)
        target_link_libraries(mameTask_tests pthread)
    endif()
    
    # Message about the build
    message(STATUS "Building for host using CMake")
endif()
