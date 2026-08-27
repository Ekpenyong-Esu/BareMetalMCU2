/**
 * @file adc_voltmeter_app.h
 * @brief The ADC application: voltmeter example
 */

#ifndef ADC_VOLTMETER_APP_H
#define ADC_VOLTMETER_APP_H

#ifdef __cplusplus
extern "C" {
#endif

/** Read the voltage on PA0 and print it over USART1, once per second. */
void AdcVoltmeterApp_Run(void);

#ifdef __cplusplus
}
#endif

#endif /* ADC_VOLTMETER_APP_H */
