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

if (TARGET pico_audio_i2s)
    add_executable(pico_braids_i2s
            pico-braids.cc
            macro_oscillator.cc
            analog_oscillator.cc
            digital_oscillator.cc
            resources.cc
            random.cc
            )

    target_link_libraries(pico_braids_i2s PRIVATE
            pico_stdlib
            pico_audio_i2s
            )

    target_compile_definitions(pico_braids_i2s PRIVATE
            # compile time configuration of I2S
            PICO_AUDIO_I2S_MONO_INPUT=1
            #define for our example code
            USE_AUDIO_I2S=1
            )
    # create map/bin/hex file etc.
    pico_add_extra_outputs(pico_braids_i2s)
endif ()

# only build PWM example if library is available
if (TARGET pico_audio_pwm)
    add_executable(pico_braids_pwm
            pico-braids.cc
            macro_oscillator.cc
            analog_oscillator.cc
            digital_oscillator.cc
            resources.cc
            random.cc
            )

    target_link_libraries(pico_braids_pwm PRIVATE
            pico_stdlib
            pico_audio_pwm
            )

    target_compile_definitions(pico_braids_pwm PRIVATE
            #define for our example code
            USE_AUDIO_PWM=1
            )
    # create map/bin/hex file etc.
    pico_add_extra_outputs(pico_braids_pwm)
endif ()
