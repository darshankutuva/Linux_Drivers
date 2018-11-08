ifeq (${KERNELRELEASE},)
	CCPREFIX := ${RASPBIAN_ROOT}/tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64/bin/arm-linux-gnueabihf-
	KERNEL_SOURCE := ${RASPBIAN_ROOT}/linux
	PWD := $(shell pwd)
default:
	${MAKE} ARCH=arm CROSS_COMPILE=${CCPREFIX} -C ${KERNEL_SOURCE} SUBDIRS=${PWD} modules
	$(CCPREFIX)gcc -g main.c -o main
clean:
	${MAKE} ARCH=arm CROSS_COMPILE=${CCPREFIX} -C ${KERNEL_SOURCE} SUBDIRS=${PWD} clean
	rm main
else
	obj-m := rw_dev.o
endif
