#include "rfid_reader.h"

volatile sig_atomic_t keep_running = 1;

void int_rfid_handler(int dummy) {
    keep_running = 0;
}

size_t rfid_read(int fd, char *const buffer, const size_t length) {
    size_t len = 0;
    int status = ETIMEDOUT;

    if (!buffer || length < 2 ) {
        return 0;
    }

    while (1) {
        struct input_event ev;
        ssize_t n;
        int digit;

        n = read(fd, &ev, sizeof ev);
        if (n == (ssize_t)-1) {
            if (errno == EINTR)
                continue;
            status = errno;
            break;
        } else if (n == sizeof ev) {
            if (ev.type != EV_KEY || (ev.value != 1 && ev.value != 2))
                continue;

            switch (ev.code) {
                case KEY_0: digit = '0'; break;
                case KEY_1: digit = '1'; break;
                case KEY_2: digit = '2'; break;
                case KEY_3: digit = '3'; break;
                case KEY_4: digit = '4'; break;
                case KEY_5: digit = '5'; break;
                case KEY_6: digit = '6'; break;
                case KEY_7: digit = '7'; break;
                case KEY_8: digit = '8'; break;
                case KEY_9: digit = '9'; break;
                default:    digit = '\0';
            }

            if (digit == '\0') {
                if (!len)
                    continue;
                status = 0;
                break;
            }

            if (len < length - 1)
                buffer[len] = digit;
            len++;
            continue;

        } else if (n == 0) {
            status = ENOENT;
            break;                
        } else {
            status = EIO;
            break;
        }
    }

    if (len < length)
        buffer[len] = '\0';
    else
        buffer[length - 1] = '\0';

    errno = status;
    return len;
}

int init_rfid_device(char *device_path_out, size_t path_len) {
    DIR *dir;
    struct dirent *ent;
    int fd = -1;
    struct input_id id;
    char name[256] = "Unknown";

    unsigned short target_vendor_id = TARGET_VENDOR_ID;
    unsigned short target_product_id = TARGET_PRODUCT_ID;

    dir = opendir("/dev/input");
    if (!dir) {
        perror("Failed to open /dev/input directory");
        return -1;
    }

    while ((ent = readdir(dir)) != NULL) {
        if (strncmp(ent->d_name, "event", 5) != 0)
            continue;

        snprintf(device_path_out, path_len, "/dev/input/%s", ent->d_name);

        fd = open(device_path_out, O_RDONLY);
        if (fd == -1) {
            continue;
        }

        if (ioctl(fd, EVIOCGID, &id) == -1) {
            close(fd);
            continue;
        }

        if (id.vendor == target_vendor_id && id.product == target_product_id) {
            printf("Found target device: %s\n", device_path_out);

            if (ioctl(fd, EVIOCGNAME(sizeof(name)), name) != -1) {
                printf("Device Name: %s\n", name);
            }

            break;
        }

        close(fd);
    }

    closedir(dir);

    if (fd == -1) {
        fprintf(stderr, "No input device found with Vendor ID 0x%04x and Product ID 0x%04x\n", 
                target_vendor_id, target_product_id);
        return -1;
    }

    if (ioctl(fd, EVIOCGRAB, 1) == -1) {
        perror("Failed to grab the device");
        close(fd);
        return -1;
    }

    printf("Device grabbed successfully.\n");
    return fd;
}

void* read_rfid_data(void* arg) {
    rfid_thread_data_t *data = (rfid_thread_data_t*)arg;
    int fd = data->fd;

    while (keep_running) {
        char code[RFID_MAXLEN + 1];
        size_t len;

        len = rfid_read(fd, code, sizeof(code));
        if (errno != 0) {
            perror("Error reading rfid");
            break;
        }
        if (len < 1) {
            fprintf(stderr, "No rfid data read.\n");
            break;
        }

        printf("rfid data: %s\n", code);
        fflush(stdout);
    }

    return NULL;
}
