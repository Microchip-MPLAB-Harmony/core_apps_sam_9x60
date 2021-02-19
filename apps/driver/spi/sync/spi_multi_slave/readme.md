---
parent: Harmony 3 driver and system service application examples for SAM 9X60 family
title: SPI Driver synchronous - multi client 
has_children: false
has_toc: false
---

[![MCHP](https://www.microchip.com/ResourcePackages/Microchip/assets/dist/images/logo.png)](https://www.microchip.com)

# SPI Driver synchronous - multi client

This example demonstrates how to use single instance of the SPI driver in synchronous mode to communicate with multiple EEPROMs in an RTOS environment.

## Description

This example writes and reads data to and from two separate EEPROMs connected over the same SPI bus by using the multi client feature of a synchronous SPI driver. The example also demonstrates how to setup two different client transfers at two different baud rates.

The example has three RTOS threads for the purpose:

- **APP_EEPROM1_Tasks**: This thread performs write-read operation on the first EEPROM
- **APP_EEPROM2_Tasks**: This thread performs write-read operation on the second EEPROM
- **APP_MONITOR_Tasks**: This thread checks the status of the EEPROM transfers and turns on the LED if the transfers are successful

## Downloading and building the application

To clone or download this application from Github, go to the [main page of this repository](https://github.com/Microchip-MPLAB-Harmony/core_apps_sam_9x60) and then click Clone button to clone this repository or download as zip file.
This content can also be downloaded using content manager by following these [instructions](https://github.com/Microchip-MPLAB-Harmony/contentmanager/wiki).

Path of the application within the repository is **apps/driver/spi/sync/spi_multi_slave/firmware** .

To build the application, refer to the following table and open the project using its IDE.

| Project Name      | Description                                    |
| ----------------- | ---------------------------------------------- |
| sam_9x60_ek_freertos.X | MPLABX project for [SAM9X60-EK Evaluation Kit](https://www.microchip.com/developmenttools/ProductDetails/DT100126) |
| sam_9x60_ek_freertos_iar.IAR/spi_multi_slave_sam_9x60_ek_freertos.eww | IAR project for [SAM9X60-EK Evaluation Kit](https://www.microchip.com/developmenttools/ProductDetails/DT100126) |
|||

## Setting up AT91Bootstrap loader

To load the application binary onto the target device, we need to use at91bootstrap loader. Refer to the [at91bootstrap loader documentation](../../../../docs/readme_bootstrap.md) for details on how to configure, build and run bootstrap loader project and use it to bootstrap the application binaries.

## Setting up the hardware

The following table shows the target hardware for the application projects.

| Project Name| Board|
|:---------|:---------:|
| sam_9x60_ek_freertos.X <br> sam_9x60_ek_freertos_iar.IAR/spi_multi_slave_sam_9x60_ek_freertos.eww | [SAM9X60-EK Evaluation Kit](https://www.microchip.com/developmenttools/ProductDetails/DT100126) |
|||

### Setting up [SAM9X60-EK Evaluation Kit](https://www.microchip.com/developmenttools/ProductDetails/DT100126)

#### Addtional hardware required

- SD Card with FAT32 file system
- Two [EEPROM 4 Click](https://www.mikroe.com/eeprom-4-click) boards


#### Setting up the SD Card

- Download harmony MPU bootstrap loader from this [location](firmware/at91bootstrap_sam_9x60_ek_freertos.X/binaries/boot.bin)
- Copy the downloaded boot loader binary( boot.bin) onto the SD card

#### Setting up the board

- Insert one [EEPROM 4 Click](https://www.mikroe.com/eeprom-4-click) board into MIKROBUS connector J14
- Connect another [EEPROM 4 Click](https://www.mikroe.com/eeprom-4-click) to J16 connector as per below Pin Connections using jumper wires

    | J16 Connector Pins  | EEPROM 4 Click pins |
    | ------------------- | ------------------- |
    | PIN 15 | HLD |
    | PIN 24 | WP  |
    | PIN 26 | CS  |
    | PIN 23 | SCK |
    | PIN 19 | SDI |
    | PIN 21 | SDO |
    | PIN 6  | GND |
    | PIN 1  | 3V3 |

- SDMMC slot used for bootloading the application is SDMMC0 (J4)
- Connect the USB port J22 on board to the computer using a micro USB cable (to enable debug com port)
- Connect the USB port J7 on board to the computer using a micro USB cable (to power the board)
- *NOTE - Reset push button is labelled as SW3*

## Running the Application

1. Build the application using its IDE
2. Copy the output binary (named 'harmony.bin') onto the SD Card (Refer to the 'Setting up hardware' section above for setting up the SD card)
3. Insert the SD card into SDMMC slot on the board (Refer to the 'Setting up hardware' section for the correct SDMMC slot)
4. Reset the board to run the application
5. The LED turns ON when the data read from the EEPROMs matches with the data written to the EEPROMs

Refer to the following table for LED name:

| Board | LED Name |
| ----- | -------- |
|  [SAM9X60-EK Evaluation Kit](https://www.microchip.com/developmenttools/ProductDetails/DT100126)  | RGB_LED(Green) |
|||