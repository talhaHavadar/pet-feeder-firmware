#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/gpio.h>
#include <linux/ioctl.h>
#include <linux/errno.h>
#include <linux/interrupt.h>

#define MODULE_NAME         "pet_feeder"
#define DEVICE_NAME         "pet_feeder"
#define PET_FEEDER_DEBUG    1

#ifdef PET_FEEDER_DEBUG
#define PF_LOG()        printk(KERN_INFO "%s: %s::%d\n", MODULE_NAME, __FUNCTION__, __LINE__)
#else
#define PF_LOG()
#endif

MODULE_DESCRIPTION("");
MODULE_AUTHOR("Talha Can Havadar <tcanhavadar@gmail.com>");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.0.0");

#define PF_IOCTL_HEY _IO('k', 0xCC)

long pet_feeder_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);

typedef struct
{
    dev_t devno;
    struct {
        unsigned short a;
        unsigned short b;
        unsigned short c;
        unsigned short d;
    } gpioPins;
    struct file_operations fops;
    struct cdev cdev;
    struct class class;
    struct device* pDevice;
} PetFeederModuleStc;

static PetFeederModuleStc sPetFeederModuleStc = {
    .fops = {
        .owner = THIS_MODULE,
        .read           = NULL,
        .write          = NULL,
        .llseek         = NULL,
        .unlocked_ioctl = pet_feeder_ioctl,
        .open           = NULL,
        .release        = NULL,
    },
    .class = {
        .name = MODULE_NAME,
        .owner = THIS_MODULE,
    },
};

long pet_feeder_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    switch (cmd)
    {
        case PF_IOCTL_HEY:
        {
            printk(KERN_INFO "%s: Hello from the ioctl bro!.\n", MODULE_NAME);
            break;
        }

        default:
        {
            return -EINVAL;
        }
    }

    return 0;
}

static int __init pet_feeder_module_init(void)
{
    PF_LOG();

    if (alloc_chrdev_region(&sPetFeederModuleStc.devno, 0, 1, DEVICE_NAME) < 0)
    {
        printk(KERN_INFO "%s: not able to allocate region for char device.\n", MODULE_NAME);
        return -1;
    }
    cdev_init(&sPetFeederModuleStc.cdev, &sPetFeederModuleStc.fops);
    sPetFeederModuleStc.cdev.owner = THIS_MODULE;

    if (class_register(&sPetFeederModuleStc.class) < 0)
    {
        printk(KERN_INFO "%s: not able to register sysfs class.\n", MODULE_NAME);
        unregister_chrdev_region(sPetFeederModuleStc.devno, 1);
        return -1;
    }

    sPetFeederModuleStc.pDevice = device_create(&sPetFeederModuleStc.class, NULL, sPetFeederModuleStc.devno, NULL, "pet_feeder-%d", 0);
    cdev_add(&sPetFeederModuleStc.cdev, sPetFeederModuleStc.devno, 1);
    //cdev_device_add(&sPetFeederModuleStc.cdev, sPetFeederModuleStc.pDevice);

    // TODO: get gpio pins from device tree node.
    sPetFeederModuleStc.gpioPins.a = 22;
    sPetFeederModuleStc.gpioPins.b = 27;
    gpio_request(sPetFeederModuleStc.gpioPins.a, "PF_GPIO_PIN_A");
    gpio_request(sPetFeederModuleStc.gpioPins.b, "PF_GPIO_PIN_B");
    gpio_direction_output(sPetFeederModuleStc.gpioPins.a, 1);
    gpio_direction_input(sPetFeederModuleStc.gpioPins.b);
    enable_irq(gpio_to_irq(sPetFeederModuleStc.gpioPins.b));



    return 0;
}

static void __exit pet_feeder_module_exit(void)
{
    PF_LOG();
    // cdev_device_del(&sPetFeederModuleStc.cdev, sPetFeederModuleStc.pDevice);
    device_destroy(&sPetFeederModuleStc.class, sPetFeederModuleStc.devno);
    class_unregister(&sPetFeederModuleStc.class);
    cdev_del(&sPetFeederModuleStc.cdev);
    unregister_chrdev_region(sPetFeederModuleStc.devno, 1);

    disable_irq(gpio_to_irq(sPetFeederModuleStc.gpioPins.b));
    gpio_free(sPetFeederModuleStc.gpioPins.a);
    gpio_free(sPetFeederModuleStc.gpioPins.b);
}

module_init(pet_feeder_module_init);
module_exit(pet_feeder_module_exit);
