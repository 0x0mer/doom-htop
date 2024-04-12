#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <string.h>
#include <math.h>
#include <signal.h>
#include <wait.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/time.h>
#include <errno.h>
#include <linux/input.h>
#include <ctype.h>

#include "doomgeneric.h"


char get_char(int i, uint8_t image_average);
void addKeyToQueue(int pressed, unsigned int keyCode);

#define KEYBOARD_DEVICE      ("/dev/input/by-path/platform-i8042-serio-0-event-kbd")
#define Y_SCALE              (4)
#define X_SCALE              (2)
#define LINE_LEN             (DOOMGENERIC_RESX/X_SCALE)
#define NUM_PROCS            (DOOMGENERIC_RESY/Y_SCALE)
#define PROCS_MALLOC_SIZE    ( 0 ) // Use this to shift the processes' place in htop

char* arr[DOOMGENERIC_RESY/Y_SCALE] = {0};
FILE *kbd;

char prev_key_map[KEY_MAX/8 + 1];    //  Create a byte array the size of the number of keys
char current_key_map[KEY_MAX/8 + 1];    //  Create a byte array the size of the number of keys


struct image_t
{
    uint8_t *img_ptr;
    int width;
    int height;
    int channels;
};

uint8_t get_pixel(struct image_t img, int x, int y, uint8_t color)
{
    if (img.width <= x || img.height <= y || img.channels <= color) {
        printf("ERROR in get_pixel!");
        exit(1);
    }
    return img.img_ptr[img.width * y * img.channels + x * img.channels + color];
}

uint8_t get_pixel_gray(struct image_t img, int x, int y)
{
    if (img.width <= x || img.height <= y) {
        printf("ERROR in get_pixel_gray!");
        exit(1);
    }
    // Formula taken from:
    // https://en.wikipedia.org/wiki/Grayscale#Luma_coding_in_video_systems
    return (0.299*get_pixel(img, x, y, 0) + 0.587*get_pixel(img, x, y, 1) + 0.114*get_pixel(img, x, y, 2));
}

uint8_t get_avg(struct image_t img, int x, int width, int y, int height)
{
    int sum = 0;
    if (img.width < (x + width) || img.height < (y + height)) {
        printf("ERROR in get_avg! x + width: %u, y + height: %u, ", x + width, y + height);
        printf("but img.width: %u, img.height: %u, img.channels: %u\n", img.width, img.height, img.channels);
        exit(1);
    }
    for (int i = y; i < y+height; i++) {
        for (int j = x; j < x+width; j++) {
            sum += get_pixel_gray(img, j, i);
        }
    }
    
    return sum / (width * height);
}

int get_line(struct image_t my_img, int y, int block_width, int block_height, char* o_line, int max_len)
{
    int imavg = get_avg(my_img, 0, my_img.width, 0, my_img.height);
    int ind = 0;
    int final_wid = block_width, final_ht = block_height;
    if (y + block_height >= my_img.height) {
            final_ht = my_img.height - y;
    }
    for (int x = 0; x < my_img.width; x+=block_width) {
        if (x + block_width >= my_img.width) {
            final_wid = my_img.width - x;
        }
        o_line[ind++] = get_char(get_avg(my_img, x, final_wid, y, final_ht), imavg);
        if(ind >= max_len) {
            return -1;
        }
    }
    return 0;
}

int create_shm(char** o_shared) {
    int shmid = 0;
    if (-1 == (shmid = shmget ( IPC_PRIVATE, 100, 0600 ))) {
        perror("oof1\n");
        exit(-1);
    }
    if(-1 == (intptr_t)(*o_shared = (char*)shmat(shmid, 0 , 0))) {
        perror("oof2\n");
        exit(-1);
    }

    shmctl(shmid, IPC_RMID, 0); // Otherwise hogs up all shm in system eventually
                                // Or maybe not, I think that was some other bug I fixed
                                // Anyway, doesn't hurt
    return 0;
}

void DG_SleepMs(uint32_t ms)
{
    usleep (ms * 1000);
}

uint32_t DG_GetTicksMs()
{
    struct timeval  tp;
    struct timezone tzp;

    gettimeofday(&tp, &tzp);

    return (tp.tv_sec * 1000) + (tp.tv_usec / 1000); /* return milliseconds */
}

void DG_SetWindowTitle(const char * title)
{

}

void DG_Init() {}

void handle_key_press(char* current_key_map, char* prev_key_map)
{
    // based on: https://stackoverflow.com/questions/3649874/how-can-i-get-the-keyboard-state-in-linux
    if (NULL == kbd)
        return;
    char RELEVANT_KEYS[] = {KEY_ENTER, KEY_ESC, KEY_LEFT, KEY_RIGHT, KEY_UP, KEY_DOWN, KEY_LEFTCTRL, KEY_RIGHTCTRL, KEY_SPACE, KEY_LEFTSHIFT, KEY_RIGHTSHIFT};
    char key;
    memset(current_key_map, 0, KEY_MAX/8 + 1);                        //  Initate the array to zero's
    ioctl(fileno(kbd), EVIOCGKEY(KEY_MAX/8 + 1), current_key_map);    //  Fill the keymap with the current keyboard state
    for (size_t i = 0; i < sizeof(RELEVANT_KEYS); i++)
    {
        key = RELEVANT_KEYS[i];
        int cur_keyb = current_key_map[key/8];                        //  The key we want (and the seven others arround it)
        int prev_keyb = prev_key_map[key/8];  
        int mask = 1 << (key % 8);                                    //  Put a one in the same column as out key state will be in;

        if ((cur_keyb & mask) && !(prev_keyb & mask)) {
            // key is pressed
            addKeyToQueue(1, key);
        } 
        if (!(cur_keyb & mask) && (prev_keyb & mask)) {
            // key is released
            addKeyToQueue(0, key);
        } 
    }
    memset(prev_key_map, 0, KEY_MAX/8 + 1);    //  Initate the array to zero's
    memcpy(prev_key_map, current_key_map, KEY_MAX/8 + 1);

}

void DG_DrawFrame() {

    struct image_t my_img = {(u_int8_t *)DG_ScreenBuffer, DOOMGENERIC_RESX, DOOMGENERIC_RESY, 4};
    int ind=NUM_PROCS - 1;
    for (int y = 0; y < DOOMGENERIC_RESY; y+=Y_SCALE) {
            get_line(my_img, y, X_SCALE, Y_SCALE, arr[ind--], DOOMGENERIC_RESX/X_SCALE);
    }
    handle_key_press(current_key_map, prev_key_map);
    
}

int main(int argc, char** argv)
{
    
    unsigned long sleep_time = 2000;
    
    for (int i = 0; i < NUM_PROCS; i++) {
        char *shared = NULL;
        create_shm(&shared);
        if (0 == fork()) {
            volatile int * please_dont_optimize_me_out = (volatile int *) malloc(i + sizeof(int) + PROCS_MALLOC_SIZE);
            while(1) {
                memcpy(argv[0], shared, LINE_LEN);
                (*please_dont_optimize_me_out) += 1;
                usleep(sleep_time);

            }
        } else {
            arr[i] = shared;
        }
    }

    doomgeneric_Create(argc, argv);

    // ls -l /dev/input/by-path
    kbd = fopen(KEYBOARD_DEVICE, "r");

    if (NULL == kbd) {
        printf("\nERROR! Could not open the keyboard device: '%s'.\n", KEYBOARD_DEVICE);
        perror("");
    }
    

    memset(current_key_map, 0, sizeof(current_key_map));    //  Initate the array to zero's
    memset(prev_key_map, 0, sizeof(prev_key_map));    //  Initate the array to zero's

    while(1)
    {
        doomgeneric_Tick();
    }
    return 0;
}
