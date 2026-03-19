Overview
========

This "Titl_Proto_ADC"  Program use the ADC in 2 channels to read the X and Y from the tilt table and controlls 2 servos.
This was put together with modification of 2 driver examples, one for ADC interrupt and the TPM two channel example

The Student will have to add the controller software to make it work.  All this provides is the shell to read the touch
pannel  X and Y and send commands to the X and Y servo.  Because each table has servos mounted differently, the direction
for the servos may be reversed from this standard program and must be checked and addapted to each specific tilt table.
 
The adc16_interrupt example shows how to use interrupt with ADC16 driver.



Toolchain supported
===================
- IAR embedded Workbench 7.80.4
- Keil MDK 5.21a
- GCC ARM Embedded 2016-5.4-q3
- Kinetis Development Studio IDE 3.2.0
- MCUXpresso0.8

Hardware requirements
=====================
- Mini/Micro USB cable
- FRDM-KL25Z board
- Personal Computer

Board settings
==============
The board should have the Tilt Table Shield plugged in.  The touch pannel can be read with out battery power,
but the servos must have about 6 to 6.5 Volts and no more.  The regulator on the shield can be adjusted


Prepare the Demo
================
1.  Connect a USB cable between the host PC and the OpenSDA USB port on the target board.
2.  Connect the battery to the Tilt Shield
3.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control

4.  Download the program to the target board.
5.  Press the green arrow to start execution.
Running the shell
================
When the example runs successfully, The measured X and Y raw positions will be displayed
in the terminal window.

~~~~~~~~~~~~~~~~~~~~~~~~~~~
Customization options
=====================
Add your Touch Correction functions for X and Y and your controller code.
