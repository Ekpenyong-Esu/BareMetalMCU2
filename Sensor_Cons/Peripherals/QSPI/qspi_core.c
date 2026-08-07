/**
  ******************************************************************************
  * @file    qspi_core.c
  * @brief   Lifecycle, identification and power control for the serial NOR flash
  ******************************************************************************
  */

#include "qspi_core.h"
#include "qspi_flash.h"
#include "qspi_hw.h"
#include "qspi_io.h"
#include "log.h"
#include <string.h>

#define QSPI_POWER_UP_DELAY_MS      10U
#define QSPI_WAKE_DELAY_MS          5U
#define QSPI_RESET_ENABLE_DELAY_MS  1U
#define QSPI_RESET_DELAY_MS         10U

typedef struct {
    uint8_t manufacturerId;
    const char *name;
} QSPI_VendorEntry_t;

static const QSPI_VendorEntry_t s_vendors[] = {
    { QSPI_MANUFACTURER_MICRON,   "Micron/ST Flash" },
    { QSPI_MANUFACTURER_WINBOND,  "Winbond Flash"   },
    { QSPI_MANUFACTURER_MACRONIX, "Macronix Flash"  },
};

/* Usable before IsInitialized is set, unlike the public entry points. */
static QSPI_StatusTypeDef QSPI_TransportReady(const QSPI_HandleStructTypeDef *hqspi)
{
    return (hqspi != NULL && hqspi->hspi != NULL) ? QSPI_OK : QSPI_INVALID_PARAM;
}

static QSPI_StatusTypeDef QSPI_ReadJedecId(QSPI_HandleStructTypeDef *hqspi, uint8_t *device_id)
{
    QSPI_ChipSelect(true);

    QSPI_StatusTypeDef status = QSPI_SendCommand(hqspi, QSPI_CMD_READ_ID);
    if (status == QSPI_OK) {
        status = QSPI_ReceiveData(hqspi, device_id, QSPI_JEDEC_ID_LENGTH);
    }

    QSPI_ChipSelect(false);

    return status;
}

static const char *QSPI_VendorName(uint8_t manufacturerId)
{
    for (size_t i = 0; i < (sizeof(s_vendors) / sizeof(s_vendors[0])); i++) {
        if (s_vendors[i].manufacturerId == manufacturerId) {
            return s_vendors[i].name;
        }
    }

    return "Unknown Flash";
}

static QSPI_StatusTypeDef QSPI_AutoDetectMemory(QSPI_HandleStructTypeDef *hqspi)
{
    uint8_t jedecId[QSPI_JEDEC_ID_LENGTH];

    QSPI_StatusTypeDef status = QSPI_ReadJedecId(hqspi, jedecId);
    if (status != QSPI_OK) {
        return status;
    }

    hqspi->MemInfo.ManufacturerID = jedecId[0];
    hqspi->MemInfo.DeviceID1 = jedecId[1];
    hqspi->MemInfo.DeviceID2 = jedecId[2];
    hqspi->MemInfo.PageSize = QSPI_PAGE_SIZE;
    hqspi->MemInfo.SectorSize = QSPI_SECTOR_SIZE;
    hqspi->MemInfo.BlockSize = QSPI_BLOCK_SIZE;
    hqspi->MemInfo.FlashSize = QSPI_FLASH_SIZE_BYTES;

    strncpy(hqspi->MemInfo.DeviceName, QSPI_VendorName(jedecId[0]),
            QSPI_DEVICE_NAME_MAX_LENGTH - 1U);
    hqspi->MemInfo.DeviceName[QSPI_DEVICE_NAME_MAX_LENGTH - 1U] = '\0';

    return QSPI_OK;
}

QSPI_ConfigTypeDef QSPI_GetDefaultConfig(void)
{
    QSPI_ConfigTypeDef config = {
        .BaudRatePrescaler = QSPI_DEFAULT_BAUDRATE_PRESCALER
    };

    return config;
}

QSPI_StatusTypeDef QSPI_Init(QSPI_HandleStructTypeDef *hqspi)
{
    if (hqspi == NULL) {
        return QSPI_INVALID_PARAM;
    }

    log_debug("QSPI: Initializing QSPI");

    memset(hqspi, 0, sizeof(*hqspi));
    hqspi->Timeout = QSPI_TIMEOUT_DEFAULT;
    hqspi->Config = QSPI_GetDefaultConfig();

    QSPI_StatusTypeDef status = QSPI_HW_InitGPIO();
    if (status != QSPI_OK) {
        return status;
    }

    status = QSPI_HW_InitSPI(hqspi);
    if (status != QSPI_OK) {
        return status;
    }

    HAL_Delay(QSPI_POWER_UP_DELAY_MS);
    (void)QSPI_ExitDeepPowerDown(hqspi);
    HAL_Delay(QSPI_WAKE_DELAY_MS);

    status = QSPI_AutoDetectMemory(hqspi);
    if (status != QSPI_OK) {
        QSPI_HW_DeInitSPI(hqspi);
        return status;
    }

    hqspi->IsInitialized = true;

    log_debug("QSPI: QSPI initialized successfully");

    return QSPI_OK;
}

QSPI_StatusTypeDef QSPI_DeInit(QSPI_HandleStructTypeDef *hqspi)
{
    QSPI_StatusTypeDef ready = QSPI_CheckReady(hqspi);
    if (ready != QSPI_OK) {
        return ready;
    }

    (void)QSPI_EnterDeepPowerDown(hqspi);
    QSPI_HW_DeInitSPI(hqspi);

    memset(hqspi, 0, sizeof(*hqspi));

    return QSPI_OK;
}

QSPI_StatusTypeDef QSPI_Configure(QSPI_HandleStructTypeDef *hqspi, const QSPI_ConfigTypeDef *config)
{
    if (hqspi == NULL || config == NULL) {
        return QSPI_INVALID_PARAM;
    }

    hqspi->Config = *config;

    if (hqspi->IsInitialized) {
        return QSPI_HW_InitSPI(hqspi);
    }

    return QSPI_OK;
}

QSPI_StatusTypeDef QSPI_Reset(QSPI_HandleStructTypeDef *hqspi)
{
    QSPI_StatusTypeDef ready = QSPI_CheckReady(hqspi);
    if (ready != QSPI_OK) {
        return ready;
    }

    QSPI_StatusTypeDef status = QSPI_SendFramedCommand(hqspi, QSPI_CMD_RESET_ENABLE);
    if (status != QSPI_OK) {
        return status;
    }

    HAL_Delay(QSPI_RESET_ENABLE_DELAY_MS);

    status = QSPI_SendFramedCommand(hqspi, QSPI_CMD_RESET);
    if (status != QSPI_OK) {
        return status;
    }

    HAL_Delay(QSPI_RESET_DELAY_MS);

    return QSPI_OK;
}

QSPI_StatusTypeDef QSPI_GetMemoryInfo(QSPI_HandleStructTypeDef *hqspi, QSPI_MemoryInfoTypeDef *memInfo)
{
    QSPI_StatusTypeDef ready = QSPI_CheckReady(hqspi);
    if (ready != QSPI_OK || memInfo == NULL) {
        return QSPI_INVALID_PARAM;
    }

    *memInfo = hqspi->MemInfo;

    return QSPI_OK;
}

QSPI_StatusTypeDef QSPI_ReadID(QSPI_HandleStructTypeDef *hqspi, uint8_t *device_id)
{
    QSPI_StatusTypeDef ready = QSPI_CheckReady(hqspi);
    if (ready != QSPI_OK || device_id == NULL) {
        return QSPI_INVALID_PARAM;
    }

    return QSPI_ReadJedecId(hqspi, device_id);
}

QSPI_StatusTypeDef QSPI_ReadUniqueID(QSPI_HandleStructTypeDef *hqspi, uint8_t *uniqueID)
{
    QSPI_StatusTypeDef ready = QSPI_CheckReady(hqspi);
    if (ready != QSPI_OK || uniqueID == NULL) {
        return QSPI_INVALID_PARAM;
    }

    const uint8_t dummy[QSPI_UNIQUE_ID_DUMMY_BYTES] = {0};

    QSPI_ChipSelect(true);

    QSPI_StatusTypeDef status = QSPI_SendCommand(hqspi, QSPI_CMD_READ_UNIQUE_ID);
    if (status == QSPI_OK) {
        status = QSPI_SendData(hqspi, dummy, sizeof(dummy));
    }
    if (status == QSPI_OK) {
        status = QSPI_ReceiveData(hqspi, uniqueID, QSPI_UNIQUE_ID_LENGTH);
    }

    QSPI_ChipSelect(false);

    return status;
}

QSPI_StatusTypeDef QSPI_EnterDeepPowerDown(QSPI_HandleStructTypeDef *hqspi)
{
    QSPI_StatusTypeDef ready = QSPI_CheckReady(hqspi);
    if (ready != QSPI_OK) {
        return ready;
    }

    return QSPI_SendFramedCommand(hqspi, QSPI_CMD_DEEP_POWER_DOWN);
}

QSPI_StatusTypeDef QSPI_ExitDeepPowerDown(QSPI_HandleStructTypeDef *hqspi)
{
    /* Runs during QSPI_Init, before the handle is marked initialized. */
    QSPI_StatusTypeDef ready = QSPI_TransportReady(hqspi);
    if (ready != QSPI_OK) {
        return ready;
    }

    QSPI_StatusTypeDef status = QSPI_SendFramedCommand(hqspi, QSPI_CMD_RELEASE_POWER_DOWN);

    HAL_Delay(QSPI_RESET_ENABLE_DELAY_MS);

    return status;
}
