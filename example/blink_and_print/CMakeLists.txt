cmake_minimum_required(VERSION 3.13)

# Include the Pico SDK
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

# Set the project name
project(blink_and_print C CXX ASM)

# Initialize the Pico SDK
pico_sdk_init()

# Add executable
add_executable(${PROJECT_NAME}
    main.cpp
)

# Add include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src
)

# Link against Pico libraries
target_link_libraries(${PROJECT_NAME}
    pico_stdlib
    pico_async_context_poll
)

set(PICO_ASYNC_CONTEXT_POLL_DEFAULT_STACK_SIZE 4096)

# Set target properties
set_target_properties(${PROJECT_NAME} PROPERTIES
    C_STANDARD 11
    CXX_STANDARD 20
)

# Enable USB output, disable UART output
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

# Create map/bin/hex/uf2 files
pico_add_extra_outputs(${PROJECT_NAME})
