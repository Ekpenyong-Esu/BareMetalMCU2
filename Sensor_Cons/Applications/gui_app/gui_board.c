/**
 * @file    gui_board.c
 * @brief   Owner of the buses and driver handles behind the GUI
 * @details Builds the bus and device configurations from the GUI_BOARD_*
 *          choices in gui_board.h, brings the drivers up in the order the
 *          hardware needs, and routes the touch EXTI edge to the driver.
 */

#include "gui_board.h"

#include "app_low_power_activity.h"
#include "fmc.h"
#include "i2c.h"
#include "ili9341.h"
#include "ltdc_core.h"
#include "ltdc_panel.h"
#include "log.h"
#include "spi.h"
#include "ts_core.h"
#include "ts_events.h"

/* Static so the drivers can keep using them after bring-up returns. */
static SPI_Bus_t s_lcdBus;
static I2C_Bus_t s_touchBus;
static ILI9341_Handle_t s_panel;
static LTDC_HandleTypeDef s_hltdc;
static LTDC_Driver_t s_ltdc;
static TS_HandleTypeDef s_touch;
static FMC_Driver_Handle_t s_sdram;

static bool s_initialized = false;
static bool s_initResult = false;

static const SPI_BusConfig_t s_lcdBusConfig = {
    .instance = GUI_BOARD_LCD_SPI,
    .sckPort = GUI_BOARD_LCD_SCK_PORT,
    .sckPin = GUI_BOARD_LCD_SCK_PIN,
    .misoPort = GUI_BOARD_LCD_MISO_PORT,
    .misoPin = GUI_BOARD_LCD_MISO_PIN,
    .mosiPort = GUI_BOARD_LCD_MOSI_PORT,
    .mosiPin = GUI_BOARD_LCD_MOSI_PIN,
    .pull = GUI_BOARD_LCD_SPI_PULL,
    .alternate = 0 /* derived from the instance */
};

static const I2C_BusConfig_t s_touchBusConfig = {
    .instance = GUI_BOARD_TOUCH_I2C,
    .sclPort = GUI_BOARD_TOUCH_SCL_PORT,
    .sclPin = GUI_BOARD_TOUCH_SCL_PIN,
    .sdaPort = GUI_BOARD_TOUCH_SDA_PORT,
    .sdaPin = GUI_BOARD_TOUCH_SDA_PIN,
    .alternate = 0 /* derived from the instance */
};

/* Matches the on-board IS42S16400J. */
static const FMC_Driver_SDRAM_Config_t s_sdramConfig = {
    .bank = GUI_BOARD_SDRAM_BANK,
    .columnBits = FMC_SDRAM_COLUMN_BITS_NUM_8,
    .rowBits = FMC_SDRAM_ROW_BITS_NUM_12,
    .dataWidth = FMC_SDRAM_MEM_BUS_WIDTH_16,
    .internalBanks = FMC_SDRAM_INTERN_BANKS_NUM_4,
    .casLatency = FMC_SDRAM_CAS_LATENCY_3,
    .clockPeriod = FMC_SDRAM_CLOCK_PERIOD_3,
    .readBurst = FMC_SDRAM_RBURST_DISABLE,
    .readPipeDelay = FMC_SDRAM_RPIPE_DELAY_1,
    .writeProtection = FMC_SDRAM_WRITE_PROTECTION_DISABLE,
    /* Read burst is off in the controller, so the device stays on single accesses. */
    .burstLength = SDRAM_MODEREG_BURST_LENGTH_1,
    .writeBurstMode = SDRAM_MODEREG_WRITEBURST_MODE_SINGLE,
    .loadToActiveDelay = 2,
    .exitSelfRefreshDelay = 7,
    .selfRefreshTime = 4,
    .rowCycleDelay = 7,
    .writeRecoveryTime = 2,
    .rpDelay = 2,
    .rcdDelay = 2};

/* Bring-up steps --------------------------------------------------------------*/

bool GUI_Board_SdramInit(void) {
    if (FMC_Driver_SDRAM_Init(&s_sdram, &s_sdramConfig) != HAL_OK) {
        log_error("BOARD: SDRAM init failed");
        return false;
    }
    return true;
}

static bool GUI_Board_PanelInit(void) {
    if (SPI_BusInit(&s_lcdBus, &s_lcdBusConfig) != SPI_OK) {
        log_error("BOARD: could not open the LCD SPI bus");
        return false;
    }

    const ILI9341_Config_t panel = {.bus = &s_lcdBus,
                                    .csPort = GUI_BOARD_LCD_CS_PORT,
                                    .csPin = GUI_BOARD_LCD_CS_PIN,
                                    .dcPort = GUI_BOARD_LCD_DC_PORT,
                                    .dcPin = GUI_BOARD_LCD_DC_PIN,
                                    .rstPort = GUI_BOARD_LCD_RST_PORT,
                                    .rstPin = GUI_BOARD_LCD_RST_PIN};

    if (!ILI9341_Init(&s_panel, &panel)) {
        log_error("BOARD: panel init failed");
        return false;
    }
    return true;
}

static bool GUI_Board_LtdcInit(void) {
    /* A handle left READY by an earlier bring-up would make HAL skip
       HAL_LTDC_MspInit, leaving the pixel clock and RGB pins as a deep sleep
       left them. Tear it down so the controller comes back fully programmed. */
    if (s_hltdc.State != HAL_LTDC_STATE_RESET) {
        (void)HAL_LTDC_DeInit(&s_hltdc);
    }

    if (LTDC_Driver_Init(&s_ltdc, &s_hltdc) != HAL_OK) {
        log_error("BOARD: LTDC driver init failed");
        return false;
    }

    const LTDC_PanelConfig_t panel = {.framebufferAddress = GUI_BOARD_FRAMEBUFFER_ADDR,
                                      .pixelFormat = GUI_BOARD_PIXEL_FORMAT,
                                      .display = LTDC_PanelDefaultsILI9341()};

    if (LTDC_PanelInit(&s_ltdc, &panel) != HAL_OK) {
        log_error("BOARD: LTDC panel init failed");
        return false;
    }
    return true;
}

bool GUI_Board_DisplayInit(void) {
    /* The panel must be in RGB mode before the LTDC starts clocking pixels at it. */
    return GUI_Board_PanelInit() && GUI_Board_LtdcInit();
}

bool GUI_Board_TouchBusOpen(void) {
    if (I2C_BusInit(&s_touchBus, &s_touchBusConfig) != I2C_OK) {
        log_error("BOARD: could not open the touch I2C bus");
        return false;
    }
    return true;
}

static bool GUI_Board_TouchInit(void) {
    if (!GUI_Board_TouchBusOpen()) {
        return false;
    }

    TS_ConfigTypeDef config = TS_GetDefaultConfig();
    config.bus = &s_touchBus;
    config.address = 0; /* STMPE811 default */
    config.intPort = GUI_BOARD_TOUCH_INT_PORT;
    config.intPin = GUI_BOARD_TOUCH_INT_PIN;
    config.displayWidth = GUI_BOARD_DISPLAY_WIDTH;
    config.displayHeight = GUI_BOARD_DISPLAY_HEIGHT;

    if (TS_Init(&s_touch, &config) != TS_OK) {
        log_error("BOARD: touch controller init failed");
        return false;
    }

    /* The touch edge is what the low-power manager counts as activity; it runs
       from interrupt context, before the controller has been serviced. */
    (void)TS_SetActivityCallback(&s_touch, APP_TouchActivity);
    return true;
}

bool GUI_Board_Init(void) {
    if (s_initialized) {
        return s_initResult;
    }
    s_initialized = true;

    /* SDRAM first: the LTDC panel init clears the framebuffer that lives in it. */
    s_initResult = GUI_Board_SdramInit() && GUI_Board_DisplayInit();
    if (!s_initResult) {
        return false;
    }

    if (!GUI_Board_TouchInit()) {
        log_error("BOARD: continuing without touch input");
    }
    return true;
}

/* Interrupt routing -----------------------------------------------------------*/

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin != GUI_BOARD_TOUCH_INT_PIN) {
        return;
    }

    /* This edge doubles as the Stop-mode wake source; a stale wakeup flag
       would make the next low-power entry return immediately. */
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);

    TS_EXTI_Callback(&s_touch);
}

/* Accessors -------------------------------------------------------------------*/

TS_HandleTypeDef *GUI_Board_Touch(void) {
    return &s_touch;
}

I2C_Bus_t *GUI_Board_TouchBus(void) {
    return &s_touchBus;
}

ILI9341_Handle_t *GUI_Board_Panel(void) {
    return &s_panel;
}

LTDC_HandleTypeDef *GUI_Board_Ltdc(void) {
    return &s_hltdc;
}

LTDC_Driver_t *GUI_Board_LtdcDriver(void) {
    return &s_ltdc;
}
