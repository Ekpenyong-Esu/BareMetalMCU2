cmake_minimum_required(VERSION 3.22)

#
# Main App
#

# Add FPU flags to match main application
# These MUST match the flags used in the main application
set(app_cpu_PARAMS
    -mcpu=cortex-m4
    -mthumb
    -mfpu=fpv4-sp-d16
    -mfloat-abi=hard  # This flag is critical - must match main app
)

# Scan for source files
set(LIBRARY_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/HSE.c")
set(LIBRARY_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/HSE.h")

add_library(HSE STATIC ${LIBRARY_SOURCES} ${LIBRARY_HEADERS})

# Add compiler options to ensure FPU consistency
target_compile_options(HSE PRIVATE
    ${app_cpu_PARAMS}
)

# Add include directories as PUBLIC to allow access from other targets
target_include_directories(HSE PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Add sources to the target
target_sources(HSE PRIVATE
    ${SRC_FILES}
)

# # refer to stm32 HAL from stm32cubemx
# target_link_libraries(app PRIVATE
#     # stm32cubemx
# )
