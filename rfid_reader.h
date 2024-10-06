#ifndef RFID_READER_H
#define RFID_READER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <signal.h>

#define DEVICE_PATH_MAX 512
#define TARGET_VENDOR_ID  0xffff
#define TARGET_PRODUCT_ID 0x0035
#define RFID_MAXLEN  16

typedef unsigned short __u16;
typedef int __s32;

struct input_event {
    struct timeval time;
    __u16 type;
    __u16 code;
    __s32 value;
};

struct input_id {
    __u16 bustype;
    __u16 vendor;
    __u16 product;
    __u16 version;
};

#define EV_KEY          0x01
#define KEY_1           2
#define KEY_2           3
#define KEY_3           4
#define KEY_4           5
#define KEY_5           6
#define KEY_6           7
#define KEY_7           8
#define KEY_8           9
#define KEY_9           10
#define KEY_0           11

#define EVIOCGID        _IOR('E', 0x02, struct input_id)
#define EVIOCGRAB       _IOW('E', 0x90, int)
#define EVIOCGNAME(len) _IOC(_IOC_READ, 'E', 0x06, len) 

extern volatile sig_atomic_t keep_running;

typedef struct {
    int fd;
} rfid_thread_data_t;

int init_rfid_device(char *device_path_out, size_t path_len);

size_t rfid_read(int fd, char *const buffer, const size_t length);

void* read_rfid_data(void* arg);

void int_rfid_handler(int dummy);

#endif
