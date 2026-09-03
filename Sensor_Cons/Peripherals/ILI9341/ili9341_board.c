/**
  ******************************************************************************
  * @file    ili9341_board.c
  * @brief   Board wiring for the ILI9341 display on STM32F429I-DISC1
  * @details Sets up the GPIOs and SPI5 exactly as the ST board support
  *          package does. Provides the real ILI9341_MspInit/DeInit that the
  *          display driver calls. In simple words: this file connects the
  *          display's control pins and SPI bus to the right MCU pins.
  */

#include "ili9341.h"
#include "spi.h"
#include "gpio.h"
#include "stm32f4xx_hal.h"

/* Local constants - match ST BSP */
#define SPI_CRC_POLY 7  /* ST BSP uses 7 */

/* LCD_RDX_PIN - used by ST BSP but we don't use it for reads in this driver */
#define LCD_RDX_PIN   GPIO_PIN_12
#define LCD_RDX_PORT  GPIOD

/* Static flag to track if LCD IO is initialized (matches ST BSP pattern) */
static uint8_t Is_LCD_IO_Initialized = 0;

bool ILI9341_MspInit(void)
{
    if (Is_LCD_IO_Initialized == 0)
    {
        Is_LCD_IO_Initialized = 1;

        /* Enable clocks */
        __HAL_RCC_SPI5_CLK_ENABLE();  /* GPIO port clocks enabled by GPIO driver */

        GPIO_InitTypeDef GPIO_InitStruct = {0};

        /* ---- LCD Control pins (per ST BSP stm32f429i_discovery.c LCD_IO_Init) ---- */

        /* WRX/DC pin (PD13) - Output Push-Pull */
        GPIO_InitStruct.Pin = ILI9341_WRX_PIN;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;  /* ST BSP uses FAST */
        GPIO_Driver_Pin_Init(ILI9341_WRX_PORT, &GPIO_InitStruct);

        /* RDX pin (PD12) - Output Push-Pull (used by ST BSP for read operations) */
        GPIO_InitStruct.Pin = LCD_RDX_PIN;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        GPIO_Driver_Pin_Init(LCD_RDX_PORT, &GPIO_InitStruct);

        /* NCS/CS pin (PC2) - Output Push-Pull */
        GPIO_InitStruct.Pin = ILI9341_CS_PIN;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        GPIO_Driver_Pin_Init(ILI9341_CS_PORT, &GPIO_InitStruct);

        /* ST BSP does a toggle: Set or Reset the control line */
        HAL_GPIO_WritePin(ILI9341_CS_PORT, ILI9341_CS_PIN, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(ILI9341_CS_PORT, ILI9341_CS_PIN, GPIO_PIN_SET);

        /* ---- SPI5 pins (per ST BSP stm32f429i_discovery.c SPIx_MspInit) ---- */

        /* NOTE: SPI5 SCK/MISO/MOSI (PF7/PF8/PF9) are configured centrally in
         * HAL_SPI_MspInit() (see Core/Src/stm32f4xx_hal_msp.c). To avoid
         * duplicate initialization and potential conflicts, do not reconfigure
         * the AF pins here. The board-specific MSP still enables clocks and
         * configures control lines (CS/WRX/RDX) above.
         */

        /* Bus settings live with the transport, in ili9341_io.c */
        if (ILI9341_IO_BusInit() != SPI_OK)
        {
            /* Let a later call retry rather than leaving the panel claimed. */
            Is_LCD_IO_Initialized = 0;
            return false;
        }
    }

    return true;
}

void ILI9341_MspDeInit(void)
{
    /* Reset pins to default state */
    HAL_GPIO_WritePin(ILI9341_CS_PORT, ILI9341_CS_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(ILI9341_WRX_PORT, ILI9341_WRX_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_RDX_PORT, LCD_RDX_PIN, GPIO_PIN_RESET);

    Is_LCD_IO_Initialized = 0;
    /* Do not de-init SPI here; leave to central SPI DeInit if needed */
}
