# Peripheral Applications

Independent, self-contained applications you can build on the **STM32F429I-DISC1**
using the drivers in this `Peripherals/` folder. Each entry lists the peripheral
driver(s) it depends on, what the app does, and the core idea you learn.

Every listed driver lives in `Peripherals/<NAME>/` and ships with a header
(`<name>.h`), an implementation (`<name>.c`), and a `*_tutorial.ipynb` walkthrough.

> Tip: keep each application in its own entry point (e.g. call it from `main()`
> behind a build option) so the apps stay independent and easy to flash one at a time.

---

## 1. Single-Peripheral Applications

Small apps that exercise exactly one driver. Great starting points.

| # | Application | Driver(s) | Description |
|---|-------------|-----------|-------------|
| 1 | Blink & Heartbeat | `LED`, `GPIO` | Toggle the on-board LEDs at a fixed rate; add breathing effect with PWM. |
| 2 | Button Event Counter | `BUTTON`, `GPIO` | Debounced press/long-press detection; count and show state changes. |
| 3 | Serial Console | `UART` | `printf`-style logging and command echo over the virtual COM port. |
| 4 | ADC Voltmeter | `ADC` | Read an analog input and report voltage over UART. |
| 5 | DAC Waveform Generator | `DAC`, `TIM` | Output sine/triangle/sawtooth waves on the DAC. |
| 6 | Tone Player | `BUZZER`, `TIM` | Play beeps and simple melodies via PWM frequency control. |
| 7 | Servo Sweep | `SERVO`, `TIM` | Drive a hobby servo through its full angular range. |
| 8 | Stepper Positioner | `STEPPER` | Rotate a stepper motor a precise number of steps in both directions. |
| 9 | Relay Scheduler | `RELAY`, `TIM` | Switch a relay on/off on a timed schedule. |
| 10 | Timer Blinky (no CPU) | `TIM` | Generate periodic events and PWM purely in hardware. |
| 11 | RNG Dice Roller | `RNG` | Produce hardware random numbers; simulate a die/lottery. |
| 12 | CRC Integrity Checker | `CRC` | Compute and verify CRC over a data buffer. |
| 13 | RTC Clock | `RTC` | Keep wall-clock time, set alarms, print timestamps. |
| 14 | Flash Key/Value Store | `FLASH` | Persist settings in internal flash across resets. |
| 15 | EEPROM Settings | `EEPROM` | Store and reload configuration in emulated EEPROM. |
| 16 | Watchdog Guard | `IWDG` / `WWDG` | Auto-reset on a hung loop; demonstrate safe refresh. |
| 17 | Low-Power Sleep Demo | `PWR`, `RTC` (the dashboard's power manager is on branch `32-lvgl-gui-app`) | Enter Sleep/Stop/Standby and wake on button/RTC. |
| 18 | Seven-Segment Counter | `SEVEN_SEGMENT` | Multiplexed digit counter (0–9999). |
| 19 | Keypad Reader | `KEYPAD` | Scan a matrix keypad and report pressed keys. |
| 20 | System Info Dump | `SYS` | Report clocks, reset cause, and core status over UART. |
| 51 | Running LED / LED Chaser | `LED`, `TIM` | Move a lit LED across several GPIO outputs with adjustable speed and direction. |
| 52 | Joystick Reader | `ADC`, `GPIO` | Read analog X/Y axes and the push switch, then report position and direction. |
| 53 | Rotary Encoder Reader | `TIM`, `GPIO` | Decode quadrature A/B signals and the push switch for menu navigation. |
| 54 | RFID Tag Reader | `SPI` + RFID module | Read an RFID tag UID and display or transmit it. |
| 63 | DC Motor Drive (L298N) | `DCMOTOR`, `TIM` | Drive a brushed DC motor via L298N H-bridge; PWM speed, direction and coast/brake control. |

---

## 2. Sensor Applications

Read a sensor and do something with the value.

| # | Application | Driver(s) | Description |
|---|-------------|-----------|-------------|
| 21 | Motion Logger | `ACCEL` / `MEMS` | Stream accelerometer/gyro data; detect tilt and taps. |
| 22 | Temperature & Humidity | `DHT` | Read a DHT sensor; report °C / %RH over UART. |
| 23 | Ultrasonic Rangefinder | `ULTRASONIC` | Measure distance with an HC-SR04 and print cm. |
| 24 | IR Proximity Switch | `IR_DISTANCE` / `IR` | Detect nearby objects and trigger an action. |
| 25 | Laser ToF Distance | `LASER_DISTANCE` | Millimeter-accurate distance with a time-of-flight sensor. |
| 26 | Microphone VU Meter | `MIC` | Sample audio level and visualize amplitude. |
| 55 | IR Obstacle Avoidance | `IR_DISTANCE` / `IR` + `LED` | Detect obstacles and indicate the avoidance direction with LEDs. |
| 56 | Sound Detection Switch | `MIC` / `ADC` + `LED` | Detect an adjustable sound threshold and toggle an LED or trigger an event. |
| 57 | Six-Axis Motion Monitor | `ACCEL` / `MEMS` | Display three-axis acceleration and three-axis gyroscope data with tilt status. |
| 58 | Female Audio Jack Monitor | `AUDIO` + `ADC` | Detect or sample an audio signal from an external female jack and show its level. |

---

## 3. Display & UI Applications

Anything that renders to a screen.

| # | Application | Driver(s) | Description |
|---|-------------|-----------|-------------|
| 27 | TFT Hello World | `ILI9341` / `LTDC` | Draw text, shapes, and colors on the on-board TFT. |
| 28 | OLED Status Screen | `SSD1306` | Small I²C OLED showing live status text. |
| 29 | Nokia 5110 Dashboard | `NOKIA5110` | Classic monochrome LCD readout. |
| 30 | Character LCD Menu | `LCD` | HD44780-style text menu navigation. |
| 31 | Touch Paint | `TOUCHSCREEN`, `XPT2046`, `ILI9341` | Draw on screen with a resistive touch panel. |
| 32 | LVGL GUI App | `LTDC`, `DMA2D`, `TOUCHSCREEN` (on branch `32-lvgl-gui-app`) | Buttons, sliders, and charts with the LVGL framework. |
| 33 | Accelerated Graphics | `DMA2D`, `LTDC` | Hardware-accelerated fills/blits and image blending. |

---

## 4. Communication Applications

Move data between chips or hosts.

| # | Application | Driver(s) | Description |
|---|-------------|-----------|-------------|
| 34 | I²C Bus Scanner | `I2C` | Enumerate all I²C device addresses on the bus. |
| 35 | SPI Loopback / Sensor | `SPI` | Full-duplex transfer test or external SPI device read. |
| 36 | UART Command Shell | `UART`, `DMA` | Interrupt/DMA + ring-buffer driven command parser. |
| 37 | CAN Node | `CAN` | Send/receive CAN frames between two boards. |
| 38 | USB Device/Host | `USB` | Enumerate as a USB device or talk to a USB peripheral. |
| 39 | Ethernet Ping/UDP | `ETH` | Bring up the MAC and exchange UDP packets. |
| 59 | RS-485 Sensor Node | `RS_COMM` + `UART` | Exchange addressed sensor messages over an RS-485 transceiver; optionally use Modbus RTU. |
| 60 | STM32 Camera to Arduino | `CAMERA` + `UART` / `SPI` | Capture a frame or stream pixel data to an Arduino using a defined packet protocol. |
| 61 | STM32 Camera to Raspberry Pi | `CAMERA` + `UART` / `SPI` / `ETH` | Send frames, snapshots, or camera-control commands to a Raspberry Pi. |

---

## 5. Memory & Storage Applications

| # | Application | Driver(s) | Description |
|---|-------------|-----------|-------------|
| 40 | SDRAM Test | `FMC` | Initialize external SDRAM and run a read/write pattern test. |
| 41 | QSPI Flash Explorer | `QSPI` | Erase/program/read external QSPI flash; memory-mapped read. |
| 42 | DMA Memory Copy | `DMA` | Benchmark memory-to-memory transfers vs. CPU copy. |
| 62 | External Memory Card Logger | `SDIO` / `SPI` + `FATFS` | Mount an SD or microSD card and append timestamped sensor records. |

---

## 6. Combined / Project-Level Applications

Multi-peripheral projects that behave like real products.

| # | Application | Driver(s) | Description |
|---|-------------|-----------|-------------|
| 43 | Weather Station | `DHT`/`MEMS` + `ILI9341` + `RTC` | Show temperature/humidity with a timestamp on the TFT. |
| 44 | Parking Sensor | `ULTRASONIC` + `BUZZER` + `LED` | Beep faster and change LED color as an object nears. |
| 45 | Data Logger | `ADC` + `RTC` + `FLASH`/`QSPI` | Periodically sample and store timestamped readings. |
| 46 | Digital Level | `ACCEL` + `ILI9341` | Bubble-level UI driven by accelerometer tilt. |
| 47 | Remote Telemetry | Sensor + `UART`/`CAN`/`ETH` | Stream live sensor data to a host or another node. |
| 48 | Touch Synth | `TOUCHSCREEN` + `DAC` + `BUZZER` | Generate tones from touch position (theremin-style). |
| 49 | Motor Control Panel | `STEPPER`/`SERVO` + `KEYPAD` + `LCD` | Command motors from a keypad with an LCD menu. |
| 50 | Low-Power Sensor Node | `PWR` + `RTC` + Sensor + `UART` | Wake on RTC, sample, transmit, sleep — battery-friendly. |

---

## Suggested Learning Path

1. **Basics:** `GPIO` → `LED` → `BUTTON` → `TIM` → `UART`
2. **Analog:** `ADC` → `DAC` → `BUZZER`/`SERVO`
3. **Buses:** `I2C` → `SPI` → sensors (`ACCEL`, `MEMS`, displays)
4. **Display/UI:** `ILI9341`/`LTDC` → `TOUCHSCREEN` → the LVGL dashboard on branch `32-lvgl-gui-app`
5. **Storage:** `FLASH` → `EEPROM` → `FMC`/`QSPI`
6. **Systems:** `RTC` → `IWDG`/`WWDG` → `PWR` → `DMA`
7. **Networking:** `CAN` → `USB` → `ETH`
8. **Capstone:** pick one project from Section 6.

---

*Board: STM32F429I-DISC1 · Each driver folder contains a `*_tutorial.ipynb` with
step-by-step guidance and API usage examples.*
