obj-m+=pwm.o

all:
	make ARCH=arm64 -C /root/allen/test7/h2_linux_sdk/ambarella/out/h2_tvu1300/kernel/linux-4.4_kernel_tvu1300_config  M=$(PWD) modules
clean:
	make -C /root/allen/test7/h2_linux_sdk/ambarella/out/h2_tvu1300/kernel/linux-4.4_kernel_tvu1300_config  M=$(PWD) clean
