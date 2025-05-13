 # FreeRTOS Kernel
 #

#  # select the port for the target MCU
#  set(FREERTOS_PORT GCC_ARM_CM3)

#  # select the heap implementation
#  set(FREERTOS_HEAP 4)

 # create the FreeRTOS configuration library
#  add_library(freertos_config INTERFACE)

#  target_include_directories(freertos_config INTERFACE
#      ${CMAKE_CURRENT_SOURCE_DIR}/Inc
#  )

 # add the FreeRTOS Kernel library
 #dd_subdirectory(ThirdParty/FreeRTOS-Kernel)
add_subdirectory(HSI_PLL)