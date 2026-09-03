/**
 * @file fmc_types.h
 * @brief Shared types for the FMC driver
 * @details FMC connects outside memory chips like SDRAM to the MCU.
 *          This file holds settings, handles and constants for that.
 *          Other files in this driver include this header.
 */

#ifndef FMC_TYPES_H
#define FMC_TYPES_H

#include "stm32f4xx.h"
#include "stm32f4xx_hal_sdram.h"
#include "stm32f4xx_hal_sram.h"
#include "stm32f4xx_hal_nand.h"
#include <stdbool.h>
#include <stdint.h>

/* FMC Memory Types */
#define FMC_DRIVER_MEMORY_NONE      0x00U
#define FMC_DRIVER_MEMORY_SDRAM     0x01U
#define FMC_DRIVER_MEMORY_NOR       0x02U
#define FMC_DRIVER_MEMORY_NAND      0x04U

/* FMC Driver Error Codes */
#define FMC_DRIVER_ERROR_NONE       0x00U
#define FMC_DRIVER_ERROR_INIT       0x01U
#define FMC_DRIVER_ERROR_CONFIG     0x02U
#define FMC_DRIVER_ERROR_OPERATION  0x04U
#define FMC_DRIVER_ERROR_PARAM      0x08U

#define FMC_SDRAM_TIMEOUT       0x1000
#define FMC_NOR_TIMEOUT         1000U
#define FMC_NAND_TIMEOUT        1000U

/* Memory layout */
#define SDRAM_BANK1_BASE_ADDR     ((uint32_t)0xC0000000)
#define SDRAM_BANK2_BASE_ADDR     ((uint32_t)0xD0000000)
#define SDRAM_DEVICE_ADDR         SDRAM_BANK2_BASE_ADDR
#define SDRAM_DEVICE_SIZE         ((uint32_t)0x800000)  /* 8 MBytes */

/* NOR/PSRAM sub-bank bases of FMC bank 1 */
#define NOR_BANK1_BASE_ADDR       ((uint32_t)0x60000000)
#define NOR_SUBBANK_SIZE          ((uint32_t)0x04000000) /* 64 MBytes */

/* SDRAM needs a refresh often or it forgets data */
#define SDRAM_REFRESH_PERIOD_MS   64U
/* Small safety margin so refresh happens a bit early */
#define SDRAM_REFRESH_MARGIN      20U
#define SDRAM_REFRESH_COUNT_MIN   41U
#define SDRAM_REFRESH_COUNT_MAX   0x1FFFU

/* SDRAM mode settings - small set used by this driver */
#define SDRAM_MODEREG_BURST_LENGTH_1             ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_LENGTH_2             ((uint16_t)0x0001)
#define SDRAM_MODEREG_BURST_LENGTH_4             ((uint16_t)0x0002)
#define SDRAM_MODEREG_BURST_LENGTH_8             ((uint16_t)0x0004)
#define SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL      ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_TYPE_INTERLEAVED     ((uint16_t)0x0008)
#define SDRAM_MODEREG_CAS_LATENCY_2              ((uint16_t)0x0020)
#define SDRAM_MODEREG_CAS_LATENCY_3              ((uint16_t)0x0030)
#define SDRAM_MODEREG_OPERATING_MODE_STANDARD    ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_PROGRAMMED ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_SINGLE     ((uint16_t)0x0200)

/**
 * @brief Settings for SDRAM chip
 * @details Tells the driver how the SDRAM is wired and how fast it is.
 *          Timing values are wait times the chip needs between steps.
 */
typedef struct {
    uint32_t bank;                    /* Which FMC bank: BANK1 or BANK2 */
    uint32_t columnBits;              /* How many bits for column address */
    uint32_t rowBits;                 /* How many bits for row address */
    uint32_t dataWidth;               /* Data bus width (8, 16 or 32 bit) */
    uint32_t internalBanks;           /* How many banks inside the chip */
    uint32_t casLatency;              /* Wait cycles before data is ready */
    uint32_t clockPeriod;             /* Clock speed setting for SDRAM */
    uint32_t readBurst;               /* Allow reading many bytes at once */
    uint32_t readPipeDelay;           /* Extra wait for read data */
    uint32_t writeProtection;         /* Block writes if enabled */
    /* Mode register - how the chip should behave */
    uint16_t burstLength;             /* How many words per burst read */
    uint16_t writeBurstMode;          /* Single or full burst for writes */
    /* Wait times - chip needs short pauses between commands */
    uint32_t loadToActiveDelay;       /* Wait after mode load before next command */
    uint32_t exitSelfRefreshDelay;    /* Wait after waking from sleep */
    uint32_t selfRefreshTime;         /* How long a row stays open */
    uint32_t rowCycleDelay;           /* Wait between row open and next row */
    uint32_t writeRecoveryTime;       /* Wait after write before next step */
    uint32_t rpDelay;                 /* Wait to close a row */
    uint32_t rcdDelay;                /* Wait between row open and column read */
} FMC_Driver_SDRAM_Config_t;

/**
 * @brief Settings for NOR flash chip
 * @details Tells the driver how the NOR chip is wired and timed.
 */
typedef struct {
    uint32_t bank;                    /* Which FMC bank: BANK1 to BANK4 */
    uint32_t dataAddressMux;          /* Share data and address lines or not */
    uint32_t memoryType;              /* Type of memory chip */
    uint32_t memoryDataWidth;         /* Data bus width */
    uint32_t burstAccessMode;         /* Allow fast burst reads */
    uint32_t waitSignalPolarity;      /* Wait pin is high or low when busy */
    uint32_t waitSignalActive;        /* When wait signal is checked */
    uint32_t writeOperation;          /* Allow writes or read-only */
    uint32_t waitSignal;              /* Use wait pin or not */
    uint32_t extendedMode;            /* Use different timing for read/write */
    uint32_t asynchronousWait;        /* Wait without clock */
    uint32_t writeBurst;              /* Allow burst writes */
    uint32_t continuousClock;         /* Keep clock running or not */
    uint32_t writeFifo;               /* Use write buffer */
    uint32_t pageSize;                /* Page size for burst */
    /* Wait times for read/write steps */
    uint32_t addressSetupTime;        /* Wait after setting address */
    uint32_t addressHoldTime;         /* How long to hold address */
    uint32_t dataSetupTime;           /* Wait for data to be ready */
    uint32_t busTurnAroundDuration;   /* Pause when switching read to write */
    uint32_t clkDivision;             /* Clock divider */
    uint32_t dataLatency;             /* Wait before data appears */
    uint32_t accessMode;              /* How the chip is accessed */
} FMC_Driver_NOR_Config_t;

/**
 * @brief Settings for NAND flash chip
 * @details Tells the driver how the NAND chip is wired and timed.
 */
typedef struct {
    uint32_t bank;                    /* Which FMC bank: BANK2 or BANK3 */
    uint32_t waitFeature;             /* Use wait pin or not */
    uint32_t memoryDataWidth;         /* Data bus width */
    uint32_t eccComputation;          /* Check for data errors or not */
    uint32_t eccPageSize;             /* Page size for error check */
    uint32_t tclrSetupTime;           /* Wait between command and read */
    uint32_t tarSetupTime;            /* Wait between address and read */
    uint32_t thold;                   /* How long to hold signals */
    uint32_t twait;                   /* Wait time during access */
    uint32_t tset;                    /* Setup time before access */
    uint32_t thiz;                    /* Time bus stays quiet after read */
    /* Chip size info - needed to know how much to read/write */
    uint32_t pageSize;                /* Bytes in one page */
    uint32_t spareAreaSize;           /* Extra bytes per page for spare use */
    uint32_t blockSize;               /* Pages in one block */
    uint32_t blockNbr;                /* Blocks in one plane */
    uint32_t planeNbr;                /* How many planes in chip */
    uint32_t planeSize;               /* Blocks per plane */
    bool     extraCommandEnable;      /* Need extra command to read page */
} FMC_Driver_NAND_Config_t;

/**
 * @brief Main handle for FMC driver
 * @details Holds all info about the connected memory chip.
 */
typedef struct {
    uint32_t memoryType;                      /* Which memory is used: SDRAM, NOR or NAND */
    SDRAM_HandleTypeDef hsdram;               /* HAL handle for SDRAM */
    SRAM_HandleTypeDef hsram;                 /* HAL handle for NOR flash */
    NAND_HandleTypeDef hnand;                 /* HAL handle for NAND flash */
    FMC_Driver_SDRAM_Config_t sdramConfig;    /* SDRAM settings */
    uint32_t baseAddress;                     /* Start address of memory on the bus */
    uint32_t deviceSize;                      /* Size of memory in bytes */
    bool initialized;                         /* True if setup is done */
    uint32_t errorCode;                       /* Last error, 0 means no error */
} FMC_Driver_Handle_t;

#endif /* FMC_TYPES_H */
