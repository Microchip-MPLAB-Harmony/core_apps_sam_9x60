---
parent: Harmony 3 driver and system service application examples for SAM 9X60 family
title: FAT filesystem using RAM Media 
has_children: false
has_toc: false
---

[![MCHP](https://www.microchip.com/ResourcePackages/Microchip/assets/dist/images/logo.png)](https://www.microchip.com)

# FAT filesystem using RAM Media

This application shows an example of using the MPLAB Harmony File System to access RAM media

## Description

Application defines below array for reserving region in RAM for Fat File system based on the Start address and size configured in MHC

```c
#define KEEP  __attribute__((keep, section(".ram_nocache"), address(DRV_MEMORY_DEVICE_START_ADDRESS)))

/* Create an array for reserving region in RAM for Fat File system */
char CACHE_ALIGN RAM_FAT_MEDIA[DRV_MEMORY_DEVICE_MEDIA_SIZE_BYTES] KEEP;
```

### File System Operations on the RAM:

- Performs a SYS_FS_FormatDisk
- Opens a **newfile.txt** on the RAM
- Write and reads back 4KB of data on **newfile.txt**
- Verifies the Data Read back

### File system layer uses the following components

- One instance of the Memory driver is used to communicate with the RAM PLIB

## Downloading and building the application

To clone or download this application from Github, go to the [main page of this repository](https://github.com/Microchip-MPLAB-Harmony/core_apps_sam_9x60) and then click Clone button to clone this repository or download as zip file.
This content can also be downloaded using content manager by following these [instructions](https://github.com/Microchip-MPLAB-Harmony/contentmanager/wiki).

Path of the application within the repository is **apps/fs/ram_fat/firmware** .

To build the application, refer to the following table and open the project using its IDE.

| Project Name      | Description                                    |
| ----------------- | ---------------------------------------------- |
| sam_9x60_curiosity.X | MPLABX project for [SAM9X60-Curiosity board](https://www.microchip.com/en-us/development-tool/EV40E67A) |
|||

## Setting up AT91Bootstrap loader

To load the application binary onto the target device, we need to use at91bootstrap loader. Refer to the [at91bootstrap loader documentation](../../docs/readme_bootstrap.md) for details on how to use it to bootstrap the application binaries.

## Setting up the hardware

The following table shows the target hardware for the application projects.

| Project Name| Board|
|:---------|:---------:|
| sam_9x60_curiosity.X | [SAM9X60-Curiosity board](https://www.microchip.com/en-us/development-tool/EV40E67A) |
|||

### Setting up [SAM9X60-Curiosity board](https://www.microchip.com/en-us/development-tool/EV40E67A)

- Connect the USB port J1 on board to the computer using a micro USB cable (to power the board)
- Connect the JTAG J12 on board to the computer using a JTAG based debugger

## Running the Application

1. Build and program the application using its IDE
2. The LED is turned ON if data has successfully been written and read back from file "newfile.txt"

Refer to the following table for LED name:

| Board | LED Name |
| ----- | -------- |
|  [SAM9X60-Curiosity board](https://www.microchip.com/en-us/development-tool/EV40E67A)  | RGB_LED(Green) |
|||