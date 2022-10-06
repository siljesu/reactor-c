# Check which system we are running on to select the correct platform support
# file and assign the file's path to LF_PLATFORM_FILE
if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(LF_PLATFORM_FILE lf_linux_support.c)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    set(LF_PLATFORM_FILE lf_macos_support.c)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    set(LF_PLATFORM_FILE lf_windows_support.c)
    set(CMAKE_SYSTEM_VERSION 10.0)
    message("Using Windows SDK version ${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}")
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Arduino")
    set(LF_PLATFORM_FILE lf_arduino_support.c)
    message("Using Arduino CMake")
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Spike")
    enable_language(ASM)
    set(LF_PLATFORM_FILE lf_spike_support.c)
    set(LF_ADDITIONAL_SOURCES core/spike/common/crt.S core/spike/common/syscall.c)
    set(CMAKE_C_COMPILER riscv32-unknown-elf-gcc)
    set(CMAKE_ASM_COMPILER riscv32-unknown-elf-gcc)
    SET(CMAKE_C_STANDARD 99)
    SET(CMAKE_BUILD_TYPE DEBUG)
    add_compile_options(-DPREALLOCATE=1 -mcmodel=medany -static -march=rv32i -mabi=ilp32 -std=gnu99 -Og -ffast-math -fno-common -fno-builtin-printf)
    add_link_options(-T ../core/spike/common/link.ld -g -static -nostartfiles -lm -lgcc)
else()
    message(FATAL_ERROR "Your platform is not supported! The C target supports Linux, MacOS and Windows.")
endif()
