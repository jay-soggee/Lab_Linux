obj-m += dev_nr.obj
RESULT = major_num_example
SRC = $(RESULT).c

all: 
	make -C ~/ESLab/kernel M=$(PWD) modules 
	aarch64-linux-gnu-gcc -o $(RESULT) $(SRC)

clean:
	make -C ~/ESLab/kernel M=$(PWD) clean 
	rm -f $(RESULT)