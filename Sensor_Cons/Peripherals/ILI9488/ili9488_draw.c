/**
 ******************************************************************************
 * @file    ili9488_draw.c
 * @brief   Pixel-level drawing primitives for the ILI9488
 ******************************************************************************
 */

#include "ili9488_draw.h"
#include "ili9488_io.h"
#include "ili9488_cmd.h"

ILI9488_StatusTypeDef ILI9488_DrawPixel(ILI9488_Handle_t *hili, uint16_t posX, uint16_t posY,
                                        uint16_t color) {
    ILI9488_StatusTypeDef status = ILI9488_OK;

    ILI9488_CHECK_HANDLE(hili);

    if (posX >= hili->width || posY >= hili->height) {
        return ILI9488_INVALID_PARAM;
    }

    status = ILI9488_IO_SetAddressWindow(hili, posX, posY, posX, posY);
    if (status != ILI9488_OK) {
        return status;
    }

    status = ILI9488_IO_WriteCommand(hili, ILI9488_CMD_MEMORY_WRITE);
    if (status != ILI9488_OK) {
        return status;
    }

    return ILI9488_IO_WritePixels(hili, color, 1U);
}
