cmake_minimum_required(VERSION 3.13)

# ---- Project setup ----
include(pico_sdk_import.cmake)
project(picoF C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# ---- Initialize the Pico SDK ----
pico_sdk_init()

# ---- OPTIONAL: Auto-generate frames.c/h from PBMs ----
# Uncomment this block if you want CMake to run pbm_to_c.py automatically
#
# add_custom_command(
#     OUTPUT ${CMAKE_CURRENT_LIST_DIR}/animationB/frames.c
#            ${CMAKE_CURRENT_LIST_DIR}/animationB/frames.h
#     COMMAND python3 ${CMAKE_CURRENT_LIST_DIR}/animationB/tools/pbm_to_c.py
#             ${CMAKE_CURRENT_LIST_DIR}/animationB/frames
#     DEPENDS ${CMAKE_CURRENT_LIST_DIR}/animationB/tools/pbm_to_c.py
#             ${CMAKE_CURRENT_LIST_DIR}/animationB/frames/*.pbm
#     WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/animationB
#     COMMENT "Converting PBM frames to C arrays for AnimationB"
# )

# ---- Source files ----
set(SOURCES
    main.c

    # Shared modules
    gfx/gfx.c
    input/input.c
    registry/registry.c
    ssd1306/ssd1306.c

    # Programs
    animationA/animation_a.c
    animationB/animation_b.c
    animationB/frames0.c
    animationC/animation_c.c
    animationC/frames.c
    dino/dino.c
    brickout/brickout.c
)

# ---- Create the executable ----
add_executable(${PROJECT_NAME}
    ${SOURCES}
)

# ---- Include directories ----
# Only add shared module folders + project root.
# Program folders are NOT added, so includes must be prefixed (e.g., "animationB/frames0.h")
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    gfx
    input
    registry
    ssd1306
)

# ---- Link libraries ----
target_link_libraries(${PROJECT_NAME}
    pico_stdlib
    hardware_gpio
    hardware_i2c
    hardware_spi
    hardware_timer
)

# ---- Enable USB output, disable UART if desired ----
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

# ---- Create UF2, bin, and hex files ----
pico_add_extra_outputs(${PROJECT_NAME})
