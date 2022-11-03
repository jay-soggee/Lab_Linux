#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.d>
#include <fcntl.h>

int main(int argc, char **argv){
    short buff;

    int dev = open("/dev/my_segment",O_RDWR);
    if(argc < 2){
        printf("Put arg 0x0000 or int\n");
        return -1;
    }

    if(dev == -1){
        printf("Opening was Possible!\n");
        return -1;
    }
    printf("Opening was successful!\n");

    if(argv[1][0] == '0' && (argv[1][1] == 'x' || argv[1][1] == 'X'))
        buff = (unsigned short)strtol(&argv[1][2], NULL, 16);
    else
        buff = (unsigned short)strtol(&argv[1][2], NULL, 10);
    
    write(dev, &buff, 2);
    close(dev);

    return 0;

}