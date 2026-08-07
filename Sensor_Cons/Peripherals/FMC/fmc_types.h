/**
 * @file fmc_types.h
 * @brief Shared vocabulary for the FMC driver
 * @details Configuration structures, handle and constants. Every other
 *          translation unit in the module depends on this header and nothing else.
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

/* SDRAM refresh: every row must be refreshed within this window */
#define SDRAM_REFRESH_PERIOD_MS   64U
/* The counter is decremented from COUNT, and the datasheet asks for a margin */
#define SDRAM_REFRESH_MARGIN      20U
#define SDRAM_REFRESH_COUNT_MIN   41U
#define SDRAM_REFRESH_COUNT_MAX   0x1FFFU

/* FMC SDRAM Mode definition register defines (minimal subset used by driver) */
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

/* SDRAM Configuration Structure - follows HAL best practices */
typedef struct {
    uint32_t bank;                    /* FMC_SDRAM_BANK1 or FMC_SDRAM_BANK2 */
    uint32_t columnBits;              /* Column address bits */
    uint32_t rowBits;                 /* Row address bits */
    uint32_t dataWidth;               /* Memory data width */
    uint32_t internalBanks;           /* Number of internal banks */
    uint32_t casLatency;              /* CAS latency */
    uint32_t clockPeriod;             /* SDRAM clock period */
    uint32_t readBurst;               /* Read burst enable/disable */
    uint32_t readPipeDelay;           /* Read pipe delay */
    uint32_t writeProtection;         /* Write protection */
    /* Mode register contents programmed into the device */
    uint16_t burstLength;             /* SDRAM_MODEREG_BURST_LENGTH_x */
    uint16_t writeBurstMode;          /* SDRAM_MODEREG_WRITEBURST_MODE_x */
    /* Timing parameters */
    uint32_t loadToActiveDelay;       /* tMRD */
    uint32_t exitSelfRefreshDelay;    /* tXSR */
    uint32_t selfRefreshTime;         /* tRAS */
    uint32_t rowCycleDelay;           /* tRC */
    uint32_t writeRecoveryTime;       /* tWR */
    uint32_t rpDelay;                 /* tRP */
    uint32_t rcdDelay;                /* tRCD */
} FMC_Driver_SDRAM_Config_t;

/* NOR Flash Configuration Structure - follows HAL best practices */
typedef struct {
    uint32_t bank;                    /* FMC_NORSRAM_BANK1 to FMC_NORSRAM_BANK4 */
    uint32_t dataAddressMux;          /* Data/Address multiplexing */
    uint32_t memoryType;              /* Memory type */
    uint32_t memoryDataWidth;         /* Data width */
    uint32_t burstAccessMode;         /* Burst access mode */
    uint32_t waitSignalPolarity;      /* Wait signal polarity */
    uint32_t waitSignalActive;        /* Wait signal active edge */
    uint32_t writeOperation;          /* Write operation enable */
    uint32_t waitSignal;              /* Wait signal enable */
    uint32_t extendedMode;            /* Extended mode enable */
    uint32_t asynchronousWait;        /* Asynchronous wait */
    uint32_t writeBurst;              /* Write burst enable */
    uint32_t continuousClock;         /* Continuous clock */
    uint32_t writeFifo;               /* Write FIFO */
    uint32_t pageSize;                /* Page size */
    /* Timing parameters */
    uint32_t addressSetupTime;        /* Address setup time */
    uint32_t addressHoldTime;         /* Address hold time */
    uint32_t dataSetupTime;           /* Data setup time */
    uint32_t busTurnAroundDuration;   /* Bus turn around duration */
    uint32_t clkDivision;             /* Clock division */
    uint32_t dataLatency;             /* Data latency */
    uint32_t accessMode;              /* Access mode */
} FMC_Driver_NOR_Config_t;

/* NAND Flash Configuration Structure - follows HAL best practices */
typedef struct {
    uint32_t bank;                    /* FMC_NAND_BANK2 or FMC_NAND_BANK3 */
    uint32_t waitFeature;             /* Wait feature enable */
    uint32_t memoryDataWidth;         /* Data width */
    uint32_t eccComputation;          /* ECC computation */
    uint32_t eccPageSize;             /* ECC page size */
    uint32_t tclrSetupTime;           /* CLE to RE delay */
    uint32_t tarSetupTime;            /* ALE to RE delay */
    uint32_t thold;                   /* Hold setup time */
    uint32_t twait;                   /* Wait setup time */
    uint32_t tset;                    /* Setup time */
    uint32_t thiz;                    /* Data bus Hi-Z setup time */
    /* Device geometry; HAL_NAND_Read/Write_Page need it to size a transfer */
    uint32_t pageSize;                /* Bytes per page */
    uint32_t spareAreaSize;           /* Bytes of spare area per page */
    uint32_t blockSize;               /* Pages per block */
    uint32_t blockNbr;                /* Blocks per plane */
    uint32_t planeNbr;                /* Number of planes */
    uint32_t planeSize;               /* Blocks per plane */
    bool     extraCommandEnable;      /* Extra command needed for page reading */
} FMC_Driver_NAND_Config_t;

/* FMC Driver Handle Structure - follows HAL best practices */
typedef struct {
    uint32_t memoryType;                      /* Type of memory (SDRAM/NOR/NAND) */
    SDRAM_HandleTypeDef hsdram;               /* SDRAM handle */
    SRAM_HandleTypeDef hsram;                 /* SRAM handle (for NOR Flash) */
    NAND_HandleTypeDef hnand;                 /* NAND handle */
    FMC_Driver_SDRAM_Config_t sdramConfig;    /* SDRAM configuration */
    uint32_t baseAddress;                     /* Base of the mapped memory window */
    uint32_t deviceSize;                      /* Size of the mapped memory window */
    bool initialized;                         /* Initialization status */
    uint32_t errorCode;                       /* Last error code */
} FMC_Driver_Handle_t;

#endif /* FMC_TYPES_H */
