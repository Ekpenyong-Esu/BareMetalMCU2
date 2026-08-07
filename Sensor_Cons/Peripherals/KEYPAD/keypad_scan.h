/**
 * @file keypad_scan.h
 * @brief Internal GPIO matrix driving and sampling for the keypad
 * @note  Not part of the public keypad.h aggregator.
 */

#ifndef KEYPAD_SCAN_H
#define KEYPAD_SCAN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "keypad_types.h"

/**
 * @brief   Configure the row outputs and column inputs
 * @param   config Pin configuration
 */
void Keypad_Scan_GpioInit(const KeypadConfig_t *config);

/**
 * @brief   Release the rows and return every line to a high impedance input
 * @param   config Pin configuration
 */
void Keypad_Scan_GpioDeInit(const KeypadConfig_t *config);

/**
 * @brief   Drive each row in turn and report the first contact found
 * @param   config Pin configuration
 * @param   row    Receives the row index of the contact
 * @param   col    Receives the column index of the contact
 * @retval  bool True if a key is held down
 */
bool Keypad_Scan_Matrix(const KeypadConfig_t *config, uint8_t *row, uint8_t *col);

#ifdef __cplusplus
}
#endif

#endif /* KEYPAD_SCAN_H */
