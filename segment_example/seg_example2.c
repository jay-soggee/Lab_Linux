#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.d>
#include <fcntl.h>
#include <termios.h>

static struct termios init_setting, new_setting;
char seg_num[10] = {0xc0, 0xf9, 0xa4, 0xb0, 0x99, 0x92, 0x82, 0xd8, 0x80, 0x90};
char seg_dnum[10] = {0x40, 0x79, 0x24, 0x30, 0x19, 0x12, 0x02, 0x58, 0x00, 0x10};

#define D1 0x01
#define D2 0x02
#define D3 0x04
#define D4 0x08

void init_keyboard(){
    tcgetattr(STDIN_FILNO, &init_setting);
    new_setting = init_setting;
    new_setting.c_lflag &= ~ICANON;
    new_setting.c_lflag &= ~ECHO;
    new_setting.c_cc[VMIN] = 0;
    new_setting.c_cc[VTIME] = 0;
    tcsetattr(0, TCSANOW, &new_setting);
}

void close_keyboard(){
    tcsetattr(0, TCSANOW, &init_setting);
}

char get_key(){
    char ch = -1;

    if(read(STDIN_FILNO, &ch, 1) != 1)  ch = -1;      
    return ch;
}

void print_menu(){
    printf("/n/n ------------------------------- /n");
    printf("[r] : Program Reset\n");
    printf("[q] : Program Exit\n");
    printf("/n/n ------------------------------- /n");
}

int main(int argc, char **argv){
    unsigned short data[4];
    char key;
    int tmp_n;
    int delay_time;

    int dev = open("/dev/my_segment", O_RDWR);
    if(dev == -1){
        printf("Opening was not Possible!\n");
        return -1;
    }
    printf("Device Opening was Successful!\n");

    init_keyboard();
    print_menu();
    tmp_n = 0;
    delay_time = 1000000;
    
    data[0] = (seg_num[1] << 4) | D1;
    data[1] = (seg_num[2] << 4) | D2;
    data[2] = (seg_num[3] << 4) | D3;
    data[3] = (seg_num[4] << 4) | D4;

    while (1){
        key = get_key();
        if(key == 'q'){
            printf("Exit this Program. \n");
            break;
        }
        else if (key == 'r'){
            delay_time = 1000000;
            tmp_n = 0;
        }

        write(dev, &data[tmp_n], 2);
        usleep(delay_time);

        tmp_n++;
        if(tmp_n > 3){
            tmp_n = 0;
            if(delay_time > 5000){
                delay_time /= 2;
            }
        }
    }
    
    close_keyboard();
    write(dev, 0x0000, 2);
    close(dev);

    return 0;
}