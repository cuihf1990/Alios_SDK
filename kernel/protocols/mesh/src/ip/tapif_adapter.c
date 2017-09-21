/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_tun.h>

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

#include <cpu_event.h>

#define DEVTAP "/dev/net/tun"
#define DEVTAP_DEFAULT_IF "tun0"

static struct {
    pthread_t th;
    int fd;
} tapif_stat = {
    .fd = -1,
};

struct databuf {
    int len;
    char buf[0];
};

void ur_adapter_input_buf(void *buf, int len);
static void pass_to_urmesh(const void* arg)
{
    struct databuf *dbuf = (struct databuf *)arg;

    ur_adapter_input_buf(dbuf->buf, dbuf->len);

    cpu_event_free(dbuf);
}

static void *tapif_recv_entry(void *arg)
{
    int fd = tapif_stat.fd;
    while (1) {
        char buf[2048];
        int len = read(fd, buf, sizeof buf);
        if (len < 0) {
            if (errno == EINTR)
                continue;
            break;
        }

        struct databuf *databuf = cpu_event_malloc(sizeof(struct databuf) + len);
        databuf->len = len;
        memcpy(databuf->buf, buf, len);
        cpu_call_handler(pass_to_urmesh, databuf);
    }
    return 0;
}

int umesh_tapif_init(const char *ifname)
{
    int fd = open(DEVTAP, O_RDWR);

    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, DEVTAP_DEFAULT_IF, sizeof(ifr.ifr_name));
    ifr.ifr_name[sizeof(ifr.ifr_name)-1] = 0;
    ifr.ifr_flags = IFF_TUN|IFF_NO_PI;

    if (ioctl(fd, TUNSETIFF, (void *) &ifr) < 0) {
        perror("tapif_init: "DEVTAP" ioctl TUNSETIFF");
        exit(1);
    }

    tapif_stat.fd = fd;
    pthread_create(&tapif_stat.th, NULL, tapif_recv_entry, NULL);

    return fd;
}

void umesh_tapif_deinit(void)
{
    close(tapif_stat.fd);
    tapif_stat.fd = -1;
    pthread_join(tapif_stat.th, NULL);
}

void umesh_tapif_send(void *buf, int len)
{
    write(tapif_stat.fd, buf+4, len-4);
}

