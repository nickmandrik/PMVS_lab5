obj-m := calc.o

KDIR :=/lib/modules/$(shell uname -r)/build

PWD := $(shell pwd)

default:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	rm *.mod.c *.o *.symvers *.ko *.order
