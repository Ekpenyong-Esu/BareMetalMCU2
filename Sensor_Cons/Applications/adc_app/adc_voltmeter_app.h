/**
 * @file adc_voltmeter_app.h
 * @brief The ADC application: multi-channel voltmeter, bare-metal super-loop
 */

#ifndef ADC_VOLTMETER_APP_H
#define ADC_VOLTMETER_APP_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Run the ADC voltmeter as a blocking super-loop (no FreeRTOS).
 * @note    Initialises everything, then samples and prints in a loop;
 *          only returns on a fatal error.
 */
void AdcVoltmeterApp_Run(void);

#ifdef __cplusplus
}
#endif

#endif /* ADC_VOLTMETER_APP_H */
