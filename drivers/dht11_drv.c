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


int gpio;
dev_t devno;
struct device_node *node;
// static int major;
static struct class *dht11_class;
static struct device *dht11_device;

struct cdev cdev = {
    .owner = THIS_MODULE
};


static void dht11_start(void){
    gpio_direction_output(gpio, 0);
    mdelay(18);
    gpio_set_value(gpio, 1);
    udelay(40);
    gpio_direction_input(gpio);
}

static int dht11_wait(void){
    int timeout_us = 200;
    while(gpio_get_value(gpio) == 1 && --timeout_us){ // 等待低电平
        udelay(1);
    }
    if(timeout_us == 0){
        return -1;
    }
    timeout_us = 200;
    while(gpio_get_value(gpio) == 0 && --timeout_us){ // 等待高电平
        udelay(1);
    }
    if(timeout_us == 0){
        return -1;
    }
    timeout_us = 200;
    
    return 0;
}

static int dht11_read_byte(unsigned char* data){
    int i;
    // int us = 0;
    int timeout_us = 200;
    while(gpio_get_value(gpio) == 1 && --timeout_us){ // 等待低电平
        udelay(1);
    }
    if(timeout_us == 0){
        return -1;
    }
    for(i = 0;i<8;i++){
        timeout_us = 300;
        while(gpio_get_value(gpio) == 0 && --timeout_us){ // 等待gao电平
            udelay(1);
        }
        if(timeout_us == 0){
            return -1;
        }
        timeout_us = 300;
        // us = 0;
        udelay(40);
        if(gpio_get_value(gpio) == 1){
            *data |= (1 << (7 - i));
            while(gpio_get_value(gpio) == 1 && --timeout_us){ // 等待di电平
                udelay(1);
            }
        }
        else{
            *data &= ~(1 << (7 - i));
        }

        // if(us > 30){
        //     *data |= (1 << (7 - i));
        // }
        // else{
        //     *data &= ~(1 << (7 - i));
        // }
        
        
    
    }
    return 0;
}

static int dht11_open(struct inode *inode,struct file *file){
    // printk(KERN_EMERG "open()\n");
    return 0;
}

static ssize_t dht11_read(struct file *file,char __user *buf,size_t count,loff_t *ppos){
    int i;
    unsigned long flags;
    unsigned char data[5];
    int ret;
    if(count != 4){
        printk(KERN_EMERG "read() err.\n");
        return -1;
    }
    local_irq_save(flags);  // 关中断
    dht11_start();
    ret = dht11_wait();
    if(ret < 0){
        printk(KERN_EMERG "dht11_wait() failed\n");
        local_irq_restore(flags);  // 开中断
        return -1;
    }

    for(i = 0;i<5;i++){
        ret = dht11_read_byte(&data[i]);
        if(ret < 0){
            printk(KERN_EMERG "dht11_read_byte() failed\n");
            local_irq_restore(flags);  // 开中断
            return -1;
        }
    }
    local_irq_restore(flags);  // 开中断
    if(data[4] != (data[0] + data[1] + data[2] + data[3])){
        printk(KERN_EMERG "checksum error\n");
        return -1;
    }

    ret = copy_to_user(buf,data,4);

    return 0;
}


struct file_operations dht11_fops = {
    .owner = THIS_MODULE,
    .open = dht11_open,
    .read = dht11_read
    // .poll = dht11_poll
};

static int dht11_probe(struct platform_device *pdev){
    int ret;
    node = pdev->dev.of_node;
    if(node == NULL){
        printk(KERN_EMERG "node == NULL\n");
        return -1;
    }
    gpio = of_get_named_gpio(node, "temp-gpios", 0);
    if(gpio == -1){
        printk(KERN_EMERG "gpio == -1\n");
        return -1;
    }
    ret = alloc_chrdev_region(&devno,0,1,pdev->name);
    if(ret < 0){
        printk(KERN_EMERG "alloc_chrdev_region() failed\n");
        return -1;
    }
    cdev_init(&cdev,&dht11_fops);
    cdev_add(&cdev,devno,1);
    dht11_class = class_create(THIS_MODULE,"dht11_class");
    dht11_device = device_create(dht11_class,NULL,devno,NULL,"dht11");

    printk(KERN_EMERG "dht11_probe()\n");
    return 0;
}

static int dht11_remove(struct platform_device *pdev){
    device_destroy(dht11_class,devno);
    class_destroy(dht11_class);
    cdev_del(&cdev);
    unregister_chrdev_region(devno,1);
    printk(KERN_EMERG "dht11_remove()\n");
    return 0;
}

struct of_device_id dht11_of_match[] = {
    {.compatible = "dht11"}
};


struct platform_driver dht11_driver = {
    .driver = {
        .name = "dht11",
        .of_match_table = dht11_of_match
    },
    .probe = dht11_probe,
    .remove = dht11_remove
};


static int dht11_init(void){  
    platform_driver_register(&dht11_driver);
    printk(KERN_EMERG "dht11_init()\n");
    return 0;
}

static void dht11_exit(void){
    platform_driver_unregister(&dht11_driver);
    printk(KERN_EMERG "dht11_exit()\n");
}



module_init(dht11_init);
module_exit(dht11_exit);
MODULE_LICENSE("GPL");