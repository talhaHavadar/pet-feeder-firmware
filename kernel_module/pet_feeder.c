#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/gpio.h>

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
        .unlocked_ioctl = NULL,
        .open           = NULL,
        .release        = NULL,
    },
    .class = {
        .name = MODULE_NAME,
        .owner = THIS_MODULE,
    },
};

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
    gpio_request(sPetFeederModuleStc.gpioPins.a, "PF_GPIO_PIN_A");
    gpio_direction_output(sPetFeederModuleStc.gpioPins.a, 1);



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


    gpio_free(sPetFeederModuleStc.gpioPins.a);
}

module_init(pet_feeder_module_init);
module_exit(pet_feeder_module_exit);
