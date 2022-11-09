#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

#define D1 0x01
#define D2 0x02
#define D3 0x04
#define D4 0x08

#define COUNT_UP(num) if(++num>9999) num = 0

#define COUNT_DOWN(num) if(--num<0) num = 9999

char seg_num[10] = {0xc0, 0xf9, 0xa4, 0xb0, 0x99, 0x92, 0x82, 0xd8, 0x80, 0x90};
char seg_dnum[10] = {0x40, 0x79, 0x24, 0x30, 0x19, 0x12, 0x02, 0x58, 0x00, 0x10};

static struct termios init_setting, new_setting;

void init_keyboard(){
    tcgetattr(STDIN_FILENO, &init_setting);
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

    if(read(STDIN_FILENO, &ch, 1) != 1)  ch = -1;      
    return ch;
}

int SetSegKeyboard(){
    tcsetattr(0, TCSANOW, &init_setting);
    char buff[5] = "    \0";
    if(read(STDIN_FILENO, &buff, 4) != 4) return -1;
    printf("Set count to %s\n", buff);
    tcsetattr(0, TCSANOW, &new_setting);
    return (buff[3] - '0') + 10 * (buff[2] - '0') + 100 * (buff[1] - '0') + 1000 * (buff[0] - '0');
}

void print_menu(){
    printf("\n-----------------------------\n\n");
    printf("[u] : Count UP\n");
    printf("[d] : Count DOWN\n");
    printf("[s] : Count SETTING\n");
    printf("[q] : Program Quit\n");
    printf("\n-----------------------------\n\n");
}

int seg_write(int dev, int num) {
    if (num > 9999 || num < 0) return -1;

    unsigned short data[4];
    static int tmp_n = 0;
    int delay_time;

    data[0] = (seg_num[num / 1000]          << 4) | D1;
    data[1] = (seg_num[(num % 1000) / 100]  << 4) | D2;
    data[2] = (seg_num[(num % 100) / 10]    << 4) | D3;
    data[3] = (seg_num[num % 10]            << 4) | D4;

    delay_time = 10;
    write(dev, &data[tmp_n], 2);
    usleep(delay_time);

    tmp_n = (tmp_n + 1) % 4;

    return 0;
}



int main() {
    char key;
    char buff[3];
    char tmp1, tmp2;
    char prev1 = 'r';
    char prev2 = 'r';
    
    int num = 0;

    int dev_g = open("/dev/my_gpio_driver", O_RDWR);
    if(dev_g == -1) {
        printf("Opening gpio was not possible!\n");
        return -1;
    }
    printf("Opening gpio was successfull!\n");

    int dev_s = open("/dev/my_segment", O_RDWR);
    if(dev_s == -1){
        printf("Opening segment was not Possible!\n");
        return -1;
    }
    printf("Device Opening segment was Successful!\n");


    init_keyboard();
    print_menu();

    while(1) {
        // keyboard
        key = get_key();
        if(key == 'q'){
            printf("Exit this Program. \n");
            break;
        }
        else if (key == 'u'){
            COUNT_UP(num); 

        }
        else if (key == 'd'){
            COUNT_DOWN(num); 
        }
        else if (key == 's'){
            num = SetSegKeyboard();
        }

        // button
        read(dev_g, &buff, 3);
        prev1 = tmp1;
        prev2 = tmp2;
        tmp1 = buff[0];
        tmp2 = buff[1];
        if (prev1 != tmp1) COUNT_UP(num);
        if (prev2 != tmp2) COUNT_DOWN(num);

        // 7-segment
        seg_write(dev_s, num);
    }

    close_keyboard();

    close(dev_g);
    close(dev_s);
    return 0;
}
