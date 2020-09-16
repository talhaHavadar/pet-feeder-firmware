#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/ioctl.h>

#define PF_IOCTL_HEY _IO('k', 0xCC)

int main()
{
    int fd = open("/dev/pet_feeder-0", O_RDWR);
    if (fd < 0)
    {
        printf("Error(%d) occured open\n", fd);
    }
    else
    {
        int res = ioctl(fd, PF_IOCTL_HEY);
        if (res < 0)
        {
            printf("Error(%d) occured ioctl\n", res);
        }
    }

    return 0;
}
