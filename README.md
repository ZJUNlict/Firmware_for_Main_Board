# Firmware_for_Main_Board

ZJUNlict [Main Board](https://github.com/ZJUNlict/Main_Board)'s Firmware for the RoboCup Soccer Small-Size League https://zjunlict.cn/ 

The firmware for the STM32H743 is designed using  [Keil uVision 5.25.2.0](http://www2.keil.com/mdk5) and [STM32CubeMX 5.2.1](https://www.st.com/en/development-tools/stm32cubemx.html).

The pin allocation and related connectivity settings can be found in file Firmware_ZJUNlict.ioc. The figure below shows the overall picture.

![](./Images/Pin_Layout.png)

The main features are:
* 400MHz (could be upgraded to 480MHz with some parameter tuning in clock configuration and timer functions)
* 4 high-resolution timers for motor PWM generation, 4 timers configured to receive encoder readings and other timers configured for booster board discharge time control, dribble motor duty control, infrared LED driver, buzzer driver and internal accurate time interrupt
* 2 SPI interface to connect with nRF24L01P or SX1280 modules, 1 SPI interface to possible IMU
* 1 IIC interface to an IO expander chip, 1 IIC interface to a high-resolution A/D chip to acquire current in each motor, 1 IIC interface to possible IMU
* 1 high baud rate UART interface to UART/USB chip for easy debug
* 1 A/D interface to verify battery and capacitors voltage

After a series of function and completeness check tests in our lab, current version will be used in RoboCup 2019. 