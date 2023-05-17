---
parent: Harmony 3 driver and system service application examples for SAM 9X60 family
title: USART driver synchronous - USART multi instance 
has_children: false
has_toc: false
---

[![MCHP](https://www.microchip.com/ResourcePackages/Microchip/assets/dist/images/logo.png)](https://www.microchip.com)

# USART driver synchronous - USART multi instance

This example echoes the received characters over the two consoles using the USART driver in synchronous mode.

## Description

This example uses the USART driver in synchronous mode RTOS environments to communicate over two consoles. It receives and echoes back the characters entered by the user on the respective console.

## Downloading and building the application

To clone or download this application from Github, go to the [main page of this repository](https://github.com/Microchip-MPLAB-Harmony/core_apps_sam_9x60) and then click Clone button to clone this repository or download as zip file.
This content can also be downloaded using content manager by following these [instructions](https://github.com/Microchip-MPLAB-Harmony/contentmanager/wiki).

Path of the application within the repository is **apps/driver/usart/sync/usart_multi_instance/firmware** .

To build the application, refer to the following table and open the project using its IDE.

| Project Name      | Description                                    |
| ----------------- | ---------------------------------------------- |
| sam_9x60_curiosity_freertos.X | MPLABX project for [SAM9X60-Curiosity board](https://www.microchip.com/en-us/development-tool/EV40E67A) |
|||

## Setting up AT91Bootstrap loader

To load the application binary onto the target device, we need to use at91bootstrap loader. Refer to the [at91bootstrap loader documentation](../../../../docs/readme_bootstrap.md) for details on how to use it to bootstrap the application binaries.

## Setting up the hardware

The following table shows the target hardware for the application projects.

| Project Name| Board|
|:---------|:---------:|
| sam_9x60_curiosity_freertos.X | [SAM9X60-Curiosity board](https://www.microchip.com/en-us/development-tool/EV40E67A) |
|||

### Setting up [SAM9X60-Curiosity board](https://www.microchip.com/en-us/development-tool/EV40E67A)

#### Addtional hardware required

- Insert [USB UART CLICK](https://www.mikroe.com/usb-uart-click) board into MIKROBUS Connector J8
- Connect the USB port J1 on board to the computer using a micro USB cable (to power the board)
- Connect the JTAG J12 on board to the computer using a JTAG based debugger
- Connect the J11 on board to the computer using a UART-FTDI cable (to enable debug com port)

## Running the Application

1. Open the Terminal application (Ex.:Tera term) on the computer for both ports
2. Connect the COM port and configure the serial settings as follows (for both ports):
    - Baud : 115200
    - Data : 8 Bits
    - Parity : None
    - Stop : 1 Bit
    - Flow Control : None
3. Build and program the application using its IDE
4. Type ten characters and observe the output on the two consoles as shown below:
    - If success, when ten characters are typed it will be echoed back on the console and LED is toggled every time the characters are echoed
    - **Console 1**

    ![output_sync_usart_multi_instance_console_1](./images/output_sync_usart_multi_instance_console_1.png)

    - **Console 2**

    ![output_sync_usart_multi_instance_console_2](./images/output_sync_usart_multi_instance_console_2.png)

Refer to the following table for LED name:

| Board | LED Name |
| ----- | -------- |
|  [SAM9X60-Curiosity board](https://www.microchip.com/en-us/development-tool/EV40E67A)  | RGB_LED(Green) |
|||