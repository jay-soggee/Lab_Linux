KDIR 	= ~/working/kernel
RESULT0 	= gray_example
RESULT1 	= camera_example
SRC0 		= $(RESULT0).c
SRC1 		= $(RESULT1).c
CCC 	= gcc

all:
	make -C $(KDIR) M=$(PWD) modules
	$(CCC) -o $(RESULT0) $(SRC0)
	$(CCC) -o $(RESULT1) $(SRC1)

clean:
	make -C $(KDIR) M=$(PWD) clean
	rm -f $(RESULT0)	
	rm -f $(RESULT1)