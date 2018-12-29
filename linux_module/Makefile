obj-m:= robosys_device_driver.o

robosys_device_driver.ko: robosys_device_driver.c
	make -C /usr/src/linux M=`pwd` modules

clean:
	make -C /usr/src/linux M=`pwd` V=1 clean
