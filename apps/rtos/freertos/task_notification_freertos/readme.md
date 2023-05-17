---
parent: Harmony 3 driver and system service application examples for SAM 9X60 family
title: FreeRTOS Task Notification 
has_children: false
has_toc: false
---

[![MCHP](https://www.microchip.com/ResourcePackages/Microchip/assets/dist/images/logo.png)](https://www.microchip.com)

# FreeRTOS Task Notification

This example application is to illustrate the FreeRTOS Task Notification feature which is used as a light weight binary semaphore.

## Description

- This demonstration creates two tasks that send notifications back and forth to each other.
  - Task2 blocks to wait for Task1 to notify and will be blocked for 500ms
  - Task1 sends a notification to Tasks2, bringing it out of the blocked state, toggles an LED
  - Task1 blocks to wait for Task2 to notify
  - Task2 sends notification to Task1, bringing it out of the blocked state

- Above steps will be repeated. i.e. an LED toggles for every 500ms

## Downloading and building the application

To clone or download this application from Github, go to the [main page of this repository](https://github.com/Microchip-MPLAB-Harmony/core_apps_sam_9x60) and then click Clone button to clone this repository or download as zip file.
This content can also be downloaded using content manager by following these [instructions](https://github.com/Microchip-MPLAB-Harmony/contentmanager/wiki).

Path of the application within the repository is **apps/rtos/freertos/task_notification_freertos/firmware** .

To build the application, refer to the following table and open the project using its IDE.

| Project Name      | Description                                    |
| ----------------- | ---------------------------------------------- |
| sam_9x60_curiosity.X | MPLABX project for [SAM9X60-Curiosity board](https://www.microchip.com/en-us/development-tool/EV40E67A) |
|||

## Setting up AT91Bootstrap loader

To load the application binary onto the target device, we need to use at91bootstrap loader. Refer to the [at91bootstrap loader documentation](../../../docs/readme_bootstrap.md) for details on how to use it to bootstrap the application binaries.

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
2. The LED toggles on success for every 500ms

Refer to the following table for LED name:

| Board | LED Name |
| ----- | -------- |
|  [SAM9X60-Curiosity board](https://www.microchip.com/en-us/development-tool/EV40E67A)  | RGB_LED(Green) |
|||
