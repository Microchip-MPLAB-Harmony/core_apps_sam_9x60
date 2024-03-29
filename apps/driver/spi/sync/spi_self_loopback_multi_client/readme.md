---
parent: Harmony 3 driver and system service application examples for SAM 9X60 family
title: SPI Driver synchronous - Self loopback multi client 
has_children: false
has_toc: false
---

[![MCHP](https://www.microchip.com/ResourcePackages/Microchip/assets/dist/images/logo.png)](https://www.microchip.com)

# SPI Driver synchronous - Self loopback multi client

This example demonstrates how to use the SPI driver in synchronous mode to achieve self-loop back between multiple clients in RTOS environment.

## Description

- This example writes and reads back the same data (self loop back) for two different clients connected over the same SPI bus by using the multi client feature of a synchronous SPI driver

- The example also demonstrates how to setup two different client transfers at two different baud rates

- The example has three RTOS threads for the purpose:
  - **APP_CLIENT1_Tasks:**
    - This thread opens the SPI driver instance and performs a continuous loop back transfer
    - If the loop back is successful, the loop back is repeated every 100 ms
    - In case of an error, the thread closes the driver and suspends itself
  - **APP_CLIENT2_Tasks:**
    - This thread opens the SPI driver instance and performs a continuous loop back transfer
    - If the loop back is successful, the loop back is repeated every 100 ms
    - In case of an error, the thread closes the driver and suspends itself
  - **APP_MONITOR_Tasks:**
    - This thread checks the status of loop back done by the two client tasks and turns on the LED if the loop back transfer status reported by both the clients is successful

## Downloading and building the application

To clone or download this application from Github, go to the [main page of this repository](https://github.com/Microchip-MPLAB-Harmony/core_apps_sam_9x60) and then click Clone button to clone this repository or download as zip file.
This content can also be downloaded using content manager by following these [instructions](https://github.com/Microchip-MPLAB-Harmony/contentmanager/wiki).

Path of the application within the repository is **apps/driver/spi/sync/spi_self_loopback_multi_client/firmware** .

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

- Use jumper wire to Connect "MOSI Pin of mikroBUS Connector" to "MISO Pin of mikroBUS Connector"
  - FLEXCOM5 MOSI signal is mapped to PA22 that is routed to "MOSI Pin of mikroBUS Connector"
  - FLEXCOM5 MISO signal is mapped to PA21 that is routed to "MISO Pin of mikroBUS Connector"
- Connect the USB port J1 on board to the computer using a micro USB cable (to power the board)
- Connect the JTAG J12 on board to the computer using a JTAG based debugger


## Running the Application

1. Build and program the application using its IDE
2. LED is turned ON on Success

Refer to the following table for LED name:

| Board | LED Name |
| ----- | -------- |
|  [SAM9X60-Curiosity board](https://www.microchip.com/en-us/development-tool/EV40E67A)  | RGB_LED(Green) |
|||