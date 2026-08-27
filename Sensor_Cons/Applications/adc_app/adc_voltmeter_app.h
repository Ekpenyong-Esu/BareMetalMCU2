/**
 * @file adc_voltmeter_app.h
 * @brief The ADC application: voltmeter example, running on FreeRTOS
 */

#ifndef ADC_VOLTMETER_APP_H
#define ADC_VOLTMETER_APP_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Run the ADC voltmeter on FreeRTOS (queue-based producer/consumer).
 * @note    Initialises everything, creates the tasks, then starts the
 *          scheduler; only returns on a fatal error.
 */
void AdcVoltmeterApp_Run(void);

#ifdef __cplusplus
}
#endif

#endif /* ADC_VOLTMETER_APP_H */
