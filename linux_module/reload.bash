#!/bin/bash

make
sudo rmmod robosys_device_driver
sudo insmod robosys_device_driver.ko
sudo chmod a+rw /dev/robosys_device0
