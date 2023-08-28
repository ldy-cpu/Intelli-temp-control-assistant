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
#include <linux/of_device.h>
#include <linux/poll.h>

#include<linux/fcntl.h>

#include <linux/pwm.h>


#include <linux/slab.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/io.h>



int gpio;
dev_t devno;
struct device_node *node;
// static int major;
static struct class *fan_class;
static struct device *fan_device;

struct pwm_device *fan_pwm;

struct cdev cdev = {
    .owner = THIS_MODULE
};


// static void level1(void){
//     gpio_set_value(gpio, 0);
//     mdelay(18);
//     gpio_set_value(gpio, 1);
//     udelay(40);
//     gpio_direction_input(gpio);
// }


static ssize_t fan_write(struct file *filp,const char __user *buf,size_t count,loff_t *ppos){
    int ret;
    char kbuf;
    if(count != 1){
        printk(KERN_EMERG "count != 1\n");
        return -1;
    }
    ret = copy_from_user(&kbuf,buf,count);
    if(ret != 0){
        printk(KERN_EMERG "copy_from_user() failed\n");
        return -1;
    }
    switch(kbuf){
        case '0':
            // ret = pwm_config(fan_pwm, 0, 43500);
            ret = pwm_config(fan_pwm, 1000000, 1000000);
    
            // printk(KERN_EMERG "level0\nperiod:%d\n",fan_pwm->state.period);
            // printk(KERN_EMERG "duty_cycle:%d\n",fan_pwm->state.duty_cycle);
            break;
        case '1':
            // ret = pwm_config(fan_pwm, 14500, 43500);
            ret = pwm_config(fan_pwm, 600000, 1000000);

            // printk(KERN_EMERG "level1\nperiod:%d\n",fan_pwm->state.period);
            // printk(KERN_EMERG "duty_cycle:%d\n",fan_pwm->state.duty_cycle);
            break;
        case '2':
            // ret = pwm_config(fan_pwm, 29000, 43500);
            ret = pwm_config(fan_pwm, 300000, 1000000);
    
            // printk(KERN_EMERG "level2\nperiod:%d\n",fan_pwm->state.period);
            // printk(KERN_EMERG "duty_cycle:%d\n",fan_pwm->state.duty_cycle);
            break;
        case '3':
            // ret = pwm_config(fan_pwm, 43500, 43500);
            ret = pwm_config(fan_pwm, 0, 1000000);

            // printk(KERN_EMERG "level3\nperiod:%d\n",fan_pwm->state.period);
            // printk(KERN_EMERG "duty_cycle:%d\n",fan_pwm->state.duty_cycle);
            break;
       
        default:
            printk(KERN_EMERG "wrong level:%c\n",kbuf);
            break;
    }
    return count;
}

struct file_operations fan_fops = {
    .owner = THIS_MODULE,
    .write = fan_write
};


static int fan_probe(struct platform_device *pdev){
    int ret;
    node = pdev->dev.of_node;
    if(!node){
        printk(KERN_EMERG "node == NULL\n");
        return -1;
    }
    // gpio = of_get_named_gpio(node,"fan-gpios",0);
    
    // if(fan_pwm == -1){
    //     printk(KERN_EMERG "fan_pwm == -1\n");
    //     return -1;
    // }
    // gpio_direction_output(gpio, 0);


    fan_pwm = of_pwm_get(node,NULL);

    if(fan_pwm == NULL){
        printk(KERN_EMERG "of_pwm_get() failed\n");
        return -1;
    }


    ret = alloc_chrdev_region(&devno,0,1,"fan");
    if(ret < 0){
        printk(KERN_EMERG "alloc_chrdev_region() failed\n");
        return -1;
    }
    cdev_init(&cdev,&fan_fops);
    cdev_add(&cdev,devno,1);
    fan_class = class_create(THIS_MODULE,"fan_class");
    fan_device = device_create(fan_class,NULL,devno,NULL,"fan");

    ret = pwm_config(fan_pwm, 1000000, 1000000);
    // ret = pwm_config(fan_pwm, 0, 43500);
    // pwm_set_polarity(fan_pwm, PWM_POLARITY_INVERSED);
    pwm_enable(fan_pwm);

    printk(KERN_EMERG "fan_probe()\n");
    return 0;
}

static int fan_remove(struct platform_device *pdev){
    device_destroy(fan_class,devno);
    class_destroy(fan_class);
    cdev_del(&cdev);
    unregister_chrdev_region(devno,1);
    pwm_disable(fan_pwm);
    printk(KERN_EMERG "fan_remove()\n");
    return 0;
}





struct of_device_id fan_of_match[] = {
    {.compatible = "fan"}
};


struct platform_driver fan_driver = {
    .driver = {
        .name = "fan",
        .of_match_table = fan_of_match
    },
    .probe = fan_probe,
    .remove = fan_remove
};

static int fan_init(void){
    platform_driver_register(&fan_driver);
    printk(KERN_EMERG "fan_init()\n");
    return 0;
}

static void fan_exit(void){
    platform_driver_unregister(&fan_driver);
    printk(KERN_EMERG "fan_exit()\n");
}


module_init(fan_init);
module_exit(fan_exit);
MODULE_LICENSE("GPL");



