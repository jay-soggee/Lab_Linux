RESULT0	= gray_example
RESULT1 = hw
SRC0	= $(RESULT0).cpp
SRC1    = $(RESULT1).cpp
CC 	= g++
PKG	= `pkg-config opencv4 --libs --cflags`

all: 
		$(CC) -o $(RESULT0) $(SRC0) $(PKG)
		$(CC) -o $(RESULT1) $(SRC1) $(PKG)

clean:
		rm -f $(RESULT0)
		rm -f $(RESULT1)

