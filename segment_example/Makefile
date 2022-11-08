obj-m += gpio_driver.o
obj-m += segment_driver.o

RESULT = seg_asgn
SRC = $(RESULT).c

all:
	make -C ~/working/kernel M=$(PWD) modules
	aarch64-linux-gnu-gcc -o $(RESULT) $(SRC)

clean:
	make -C ~/working/kerenl M=$(PWD) clean
	rm -f $(RESULT)

