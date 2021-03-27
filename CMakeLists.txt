cmake_minimum_required(VERSION 3.12)

set(NAME pico-braids) # <-- Name your project/executable here!

# Pull in PICO SDK (must be before project)
include(pico_sdk_import.cmake)

# We also need PICO EXTRAS
include(pico_extras_import.cmake)

project(pico_playground C CXX)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the Pico SDK
pico_sdk_init()

add_executable(${NAME}
    pico-braids.cc
    macro_oscillator.cc
    analog_oscillator.cc
    digital_oscillator.cc
    resources.cc
    random.cc
)
target_link_libraries(${NAME} PRIVATE
pico_stdlib
pico_audio_i2s
)
target_compile_definitions(${NAME} PRIVATE
# compile time configuration of I2S
PICO_AUDIO_I2S_MONO_INPUT=1
#define for our example code
USE_AUDIO_I2S=1
)
# create map/bin/hex file etc.
pico_add_extra_outputs(${NAME})

