# Minimal import helper: relies on PICO_SDK_PATH env or cache var
if (DEFINED ENV{PICO_SDK_PATH} AND NOT PICO_SDK_PATH)
    set(PICO_SDK_PATH $ENV{PICO_SDK_PATH})
endif()

if (NOT PICO_SDK_PATH)
    message(FATAL_ERROR "PICO_SDK_PATH is not set. Point it to the Pico SDK root.")
endif()

if (NOT EXISTS "${PICO_SDK_PATH}/pico_sdk_init.cmake")
    message(FATAL_ERROR "pico_sdk_init.cmake not found at ${PICO_SDK_PATH}. Is the SDK installed correctly?")
endif()

include(${PICO_SDK_PATH}/pico_sdk_init.cmake)
