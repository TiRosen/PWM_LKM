# PWM Module for Linux Kernel on a Raspberry Pi 3b

This repository contains a simple Linux Kernel Module (LKM) for PWM (Pulse Width Modulation) control. The module is designed to work on Raspberry Pi 3b, providing basic functionality for configuring and controlling PWM signals.

Tested on Linux Kernel version 6.1.0-rpi4-rpi-v7.

# Preparation Steps

## 1. Update Package Lists
```bash
sudo apt update
```
## 2. Install Raspberry Pi Kernel Headers
```bash
sudo apt install raspberrypi-kernel-headers
```
## 3. Upgrade Installed Packages
```bash
sudo apt upgrade
```
## 4. Build the Kernel Module
  Navigate to the directory containing your .c file and the accompanying Makefile.
  Use the make command to build the .ko file.
Example:
```bash
make
```
## 5. Load the Kernel Module
```bash
sudo insmod pwm_driver.ko
```
## 6. Set Appropriate Permissions
```bash
sudo chmod 666 /dev/my_pwm_0_driver
```
## 7. Control PWM
  Send duty cycle values (0-100) to the driver.
  Example:
```bash
echo 50 > /dev/my_pwm_0_driver
```
## 8. Unload the Kernel Module
```bash
sudo rmmod pwm_driver.ko
```
## Features

- User-space interface for controlling PWM through a character device file.
- Designed for Raspberry Pi 3b.
