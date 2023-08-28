#include<linux/init.h>
#include<linux/module.h>
#include<linux/fs.h>
#include<linux/interrupt.h>
#include<linux/sched.h>
#include<linux/pm.h>
#include<linux/slab.h>
#include<linux/sysctl.h>
#include<linux/proc_fs.h>
#include<linux/delay.h>
#include<linux/interrupt.h>
#include<linux/platform_device.h>
#include<linux/of.h>
#include<linux/of_gpio.h>
#include<linux/gpio.h>
#include<linux/gpio/consumer.h>
#include<linux/irq.h>
#include<linux/irqdomain.h>
#include<linux/cdev.h>
#include<linux/device.h>
#include<linux/uaccess.h>

#include <linux/poll.h>

#include<linux/fcntl.h>

#include<linux/input.h>

struct device_node *node;
int gpio;

static struct input_dev *pir_input_device;
int irqno;




static irqreturn_t irq_handler(int irqno, void *dev_id){

    // printk(KERN_EMERG "irq_handler\n");
    if(gpio_get_value(gpio) == 1){
        // printk(KERN_EMERG "input 1\n");
        input_event(pir_input_device, EV_KEY, KEY_5, 1);
        input_sync(pir_input_device);
    }
    else{
        // printk(KERN_EMERG "input 0\n");
        input_event(pir_input_device, EV_KEY, KEY_5, 0);
        input_sync(pir_input_device);
    }
    

    return IRQ_HANDLED;
}
static int pir_probe(struct platform_device* pdev){
    int ret;
    int irqret;
    node = pdev->dev.of_node;
    if(node == NULL){
        printk(KERN_EMERG "node == NULL\n");
        return -1;
    }
    gpio = of_get_named_gpio(node, "pir-gpios", 0);
    if(gpio == -1){
        printk(KERN_EMERG "gpio == -1\n");
        return -1;
    }
    gpio_direction_input(gpio);
    irqno = gpio_to_irq(gpio);
    irqret = request_irq(irqno, irq_handler, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "pir", &gpio);


    pir_input_device = devm_input_allocate_device(&pdev->dev);
    pir_input_device->name = "pir";
    pir_input_device->phys = "pir";

    __set_bit(EV_KEY, pir_input_device->evbit);
    __set_bit(EV_SYN, pir_input_device->evbit);

    __set_bit(KEY_5, pir_input_device->keybit);

    ret = input_register_device(pir_input_device);
    printk(KERN_EMERG "pir_probe\n");
    return 0;
}

static int pir_remove(struct platform_device *pdev){
    
    input_unregister_device(pir_input_device);
    input_free_device(pir_input_device);
    free_irq(irqno, &gpio);

    printk(KERN_EMERG "pir_remove\n");
    return 0;
}

struct of_device_id pir_of_match[] = {
    {.compatible = "pir"}
};


struct platform_driver pir_driver = {
    .driver = {
        .name = "pir",
        .owner = THIS_MODULE,
        .of_match_table = pir_of_match
    },
    .probe = pir_probe,
    .remove = pir_remove
};


static int pir_init(void){
    platform_driver_register(&pir_driver);
    printk(KERN_EMERG "pir_init\n");
    return 0;
}

static void pir_exit(void){
    platform_driver_unregister(&pir_driver);
    printk(KERN_EMERG "pir_exit\n");
}

module_init(pir_init);
module_exit(pir_exit);

MODULE_LICENSE("GPL");