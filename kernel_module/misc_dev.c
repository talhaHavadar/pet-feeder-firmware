#include <linux/device.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/time.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define DEVICE_NAME     "mtestdev"

static struct gpio gpios[] = {
    {27, GPIOF_IN, "MTEST_GPIO_IN"},
    {22, GPIOF_OUT_INIT_HIGH, "MTEST_GPIO_OUT"},
};
static int gpio_irq_no = -1;

MODULE_DESCRIPTION("");
MODULE_AUTHOR("Talha Can Havadar <tcanhavadar@gmail.com>");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.0.0");

static irqreturn_t irqHandler(int irq, void *data)
{

    printk(KERN_INFO "Interrupt occured!!\n");

    return IRQ_HANDLED;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open       = NULL,
    .read       = NULL,
    .write      = NULL,
    .release    = NULL,
};

static struct miscdevice mtestdevice = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = DEVICE_NAME,
    .fops = &fops,
};

static int __init mtestdev_init(void)
{
    int ret = 0;

    if (gpio_request_array(gpios, ARRAY_SIZE(gpios)))
    {
        printk(KERN_ERR "%s: not able to request GPIOS for device.\n", DEVICE_NAME);
        return -1;
    }

    if ( (gpio_irq_no = gpio_to_irq(gpios[0].gpio) ) < 0)
    {
        printk(KERN_ERR "%s: not able to request GPIO irq for device.\n", DEVICE_NAME);
        gpio_free_array(gpios, ARRAY_SIZE(gpios));
        return -1;
    }

    if (request_irq(gpio_irq_no, irqHandler, IRQF_TRIGGER_RISING, "mtestdevice#in", NULL) < 0)
    {
        printk(KERN_ERR "%s: not able to request irq for device.\n", DEVICE_NAME);
        free_irq(gpio_irq_no, NULL);
        gpio_free_array(gpios, ARRAY_SIZE(gpios));
        return -1;
    }

    misc_register(&mtestdevice);

    return 0;

}

static void __exit mtestdev_exit(void)
{
    misc_deregister(&mtestdevice);
    free_irq(gpio_irq_no, NULL);
    gpio_free_array(gpios, ARRAY_SIZE(gpios));
}

module_init(mtestdev_init);
module_exit(mtestdev_exit);
