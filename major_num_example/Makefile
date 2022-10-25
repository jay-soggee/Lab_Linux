obj-m += dev_nr.o
RESULT = major_num_example
SRC = $(RESULT).c

all: 
	make -C ~/working/kernel M=$(PWD) modules 
	aarch64-linux-gnu-gcc -o $(RESULT) $(SRC)

clean:
	make -C ~/working/kernel M=$(PWD) clean 
	rm -f $(RESULT)
