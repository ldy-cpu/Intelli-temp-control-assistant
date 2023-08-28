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
// dev_t devno;
// static struct class *irda_class;
// static struct device *irda_device;
static struct input_dev *irda_input_device;
int irqno;
int irda_edge_time[100];
int irda_edge_count = 0;


// struct cdev cdev = {
//     .owner = THIS_MODULE
// };

// struct fasync_struct *async_queue;


int bit = 31;
unsigned int data;


//-1 error  0 success
// int parse_data(void){
    // if(irda_edge_count == 2){
    //     if(irda_edge_time[1] - irda_edge_time[0] < 10000000 && irda_edge_time[1] - irda_edge_time[0] > 8000000){ 
    //         if(irda_edge_time[2]-irda_edge_time[1] < 3000000 && irda_edge_time[2]-irda_edge_time[1] > 1750000){ //repeat
    //             irda_edge_count = 0;
    //             irda_edge_time[0] = irda_edge_time[2];
    //             return 0;
    //         }
    //         if(irda_edge_time[2]-irda_edge_time[1] < 5000000 && irda_edge_time[2]-irda_edge_time[1] > 4000000){ // start
    //             return 0;
    //         }

    //         // error set default
    //         // printk(KERN_EMERG "irda_edge_time[2]-irda_edge_time[1]:%d\n", irda_edge_time[2]-irda_edge_time[1]);

    //         return -1;
    //     }
    //     // error set default
    //     // printk(KERN_EMERG "irda_edge_time[1]-irda_edge_time[0]:%d\n", irda_edge_time[1]-irda_edge_time[0]);
        
    //     return -1;
    // }

    // if(irda_edge_count%2 == 0 && irda_edge_count >= 4){ //normal bit
    //     if(irda_edge_time[irda_edge_count-1] - irda_edge_time[irda_edge_count-2] > 400000 && irda_edge_time[irda_edge_count-1] - irda_edge_time[irda_edge_count-2] < 700000){ //low normal
    //         if(irda_edge_time[irda_edge_count] - irda_edge_time[irda_edge_count-1] > 1550000 && irda_edge_time[irda_edge_count] - irda_edge_time[irda_edge_count-1] < 1800000){ //bit 1
    //             data |= (1 << bit);
    //             bit--;
    //             return 0;
    //         }
    //         if(irda_edge_time[irda_edge_count] - irda_edge_time[irda_edge_count-1] > 400000 && irda_edge_time[irda_edge_count] - irda_edge_time[irda_edge_count-1] < 700000){ //bit 0
    //             data &= ~(1 << bit); 
    //             bit--;
    //             return 0;
    //         }

    //         // error set default
    //         // printk(KERN_EMERG "normal bit high:%d\n", irda_edge_time[irda_edge_count]-irda_edge_time[irda_edge_count - 1]);
    //         return -1;
    //     }
        

    //     if(irda_edge_time[irda_edge_count-1] - irda_edge_time[irda_edge_count-2] > 8000000 && irda_edge_time[irda_edge_count-1] - irda_edge_time[irda_edge_count-2] < 10000000){ //more tap
    //         if(irda_edge_time[irda_edge_count]-irda_edge_time[irda_edge_count-1] < 3000000 && irda_edge_time[irda_edge_count]-irda_edge_time[irda_edge_count-1] > 1750000){ //repeat
    //             irda_edge_count = 0;
    //             irda_edge_time[0] = irda_edge_time[2];
    //             bit = 31;
    //             return 0;
    //         }
    //         if(irda_edge_time[irda_edge_count]-irda_edge_time[irda_edge_count - 1] < 5000000 && irda_edge_time[2]-irda_edge_time[1] > 4000000){ // start
    //             bit = 31;
    //             irda_edge_time[2] = irda_edge_time[irda_edge_count];
    //             irda_edge_count = 2;
    //             return 0;
    //         }

    //         // error set default
    //         // printk(KERN_EMERG "more tap high:%d\n", irda_edge_time[irda_edge_count]-irda_edge_time[irda_edge_count - 1]);
    //         return -1;
    //     }

    //     // error set default
    //     // printk(KERN_EMERG "normal bit low:%d\n", irda_edge_time[irda_edge_count-1]-irda_edge_time[irda_edge_count - 2]);


    //     return -1;
    // }


    
//     return 0;
// }

void parse_data(void){
    int low;
    int high;

    if(irda_edge_count < 2)
        return;

    low = irda_edge_time[irda_edge_count-1] - irda_edge_time[irda_edge_count-2];
    high = irda_edge_time[irda_edge_count] - irda_edge_time[irda_edge_count-1];



    if(irda_edge_count > 2){
        if(low < 10000000 && low > 8000000 && high < 5000000 && high > 4000000){ // start
            bit = 31;
            irda_edge_time[2] = irda_edge_time[irda_edge_count];
            irda_edge_count = 2;
            return;
        }

        if(irda_edge_count%2 != 0){
            return;
        }

        if(low < 650000 && low > 450000 && high < 1800000 && high > 1550000){ //bit 1
            data |= (1 << bit);
            
        }
        if(low < 650000 && low > 450000 && high < 700000 && high > 400000){ //bit 0
            data &= ~(1 << bit); 
            
        }
        bit--;
        return;
    }

    // normal start
    bit = 31;
}

static irqreturn_t irq_handler(int irqno, void *dev_id){

    // int timer1;
    // int timer2;

    // int ret;
    int value;
    int value_com;
    int addr;
    int addr_com;

    // timer1 = ktime_get_boot_ns();

    irda_edge_time[irda_edge_count] = ktime_get_boot_ns();
    
    parse_data();
    irda_edge_count++;
    // printk(KERN_EMERG "irda_edge_count:%d\n", irda_edge_count);
    // if(ret == -1){
    //     // printk(KERN_EMERG "parse_data() error\n");
    //     irda_edge_count = 0;
    //     bit = 31;
    //     timer2 = ktime_get_boot_ns();
    //     // printk(KERN_EMERG "time:%d\n", timer2 - timer1);
    //     return IRQ_HANDLED;
    // }
    if(bit == -1){ //data receive complete
        // printk(KERN_EMERG "data:%d\n", data);

        addr = data >> 24;
        addr_com = (~(data >> 16)) & 0xff;
        value = data >> 8 & 0xff;
        value_com = (~(data)) & 0xff;

        if(addr == addr_com && value == value_com){ //jiao yan tong guo
            if(addr == 0x00){
                if(value == 162){
                    input_event(irda_input_device, EV_KEY,KEY_0, 1);
                    input_event(irda_input_device, EV_KEY,KEY_0, 0);
                    // input_sync(irda_input_device);
                    // printk(KERN_EMERG "switch\n");
                }
                if(value == 194){
                    input_event(irda_input_device, EV_KEY,KEY_1, 1);
                    input_event(irda_input_device, EV_KEY,KEY_1, 0);
                    // input_sync(irda_input_device);
                    // printk(KERN_EMERG "back\n");
                }
                if(value == 2){
                    input_event(irda_input_device, EV_KEY,KEY_2, 1);
                    input_event(irda_input_device, EV_KEY,KEY_2, 0);
                    // input_sync(irda_input_device);
                    // printk(KERN_EMERG "up\n");
                }
                if(value == 168){
                    input_event(irda_input_device, EV_KEY,KEY_3, 1);
                    input_event(irda_input_device, EV_KEY,KEY_3, 0);
                    // input_sync(irda_input_device);
                    // printk(KERN_EMERG "select\n");
                }
                if(value == 152){
                    input_event(irda_input_device, EV_KEY,KEY_4, 1);
                    input_event(irda_input_device, EV_KEY,KEY_4, 0);
                    // input_sync(irda_input_device);
                    // printk(KERN_EMERG "down\n");
                }
                input_sync(irda_input_device);
            }   
            
            else{
                printk(KERN_EMERG "addr_id error\n");
            }
            // printk(KERN_EMERG "addr:%d\n", addr);
            // printk(KERN_EMERG "value:%d\n", value);
        }
        else{
            printk(KERN_EMERG "jiao yan shi bai\n");
            printk(KERN_EMERG "addr:%d\n", addr);
            printk(KERN_EMERG "value:%d\n", value);
        }

        
        irda_edge_count = 0;
        bit = 31;
        // timer2 = ktime_get_boot_ns();
        // printk(KERN_EMERG "time:%d\n", timer2 - timer1);
        return IRQ_HANDLED;
    }
    
    // timer2 = ktime_get_boot_ns();
    // printk(KERN_EMERG "time:%d\n", timer2 - timer1);
    return IRQ_HANDLED;
}

// static int irda_fasync(int fd, struct file *filp, int mode){
//     int ret = fasync_helper(fd, filp, mode, &async_queue);
//     printk(KERN_EMERG "irda_fasync()\n");
//     return ret;
// }

// struct file_operations irda_fops = {
//     .owner = THIS_MODULE,
//     .fasync = irda_fasync
// };

static int irda_probe(struct platform_device *pdev){
    int ret;
    int irqret;
    node = pdev->dev.of_node;
    if(node == NULL){
        printk(KERN_EMERG "node == NULL\n");
        return -1;
    }
    gpio = of_get_named_gpio(node, "irda-gpios", 0);
    if(gpio == -1){
        printk(KERN_EMERG "gpio == -1\n");
        return -1;
    }
    gpio_direction_input(gpio);
    irqno = gpio_to_irq(gpio);
    irqret = request_irq(irqno, irq_handler, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "irda", &gpio);


    // ret = alloc_chrdev_region(&devno,0,1,pdev->name);
    // if(ret < 0){
    //     printk(KERN_EMERG "alloc_chrdev_region() failed\n");
    //     return -1;
    // }
    // cdev_init(&cdev,&irda_fops);
    // cdev_add(&cdev,devno,1);
    // irda_class = class_create(THIS_MODULE,"irda_class");
    // irda_device = device_create(irda_class,NULL,devno,NULL,"irda");

///////////////////////////////////////////////////////////////////////////////// input sys
    irda_input_device = devm_input_allocate_device(&pdev->dev);

    irda_input_device->name = "irda";
    irda_input_device->phys = "irda";

    //event type
    __set_bit(EV_KEY, irda_input_device->evbit);
    // __set_bit(EV_REP, irda_input_device->evbit);

    //event
    __set_bit(KEY_0, irda_input_device->keybit);//switch
    __set_bit(KEY_1, irda_input_device->keybit);//back
    __set_bit(KEY_2, irda_input_device->keybit);//up
    __set_bit(KEY_3, irda_input_device->keybit);//select
    __set_bit(KEY_4, irda_input_device->keybit);//down


    ret = input_register_device(irda_input_device);
    
//////////////////////////////////////////////////////////////////////////////////////

    printk(KERN_EMERG "probe()\n");
    return 0;
}

static int irda_remove(struct platform_device *pdev){

    input_unregister_device(irda_input_device);
    input_free_device(irda_input_device);

    free_irq(irqno, &gpio);

    printk(KERN_EMERG "remove()\n");
    return 0;
}

struct of_device_id irda_of_match[] = {
    {.compatible = "irda"}
};

struct platform_driver irda_driver = {
    .driver = {
        .name = "irda",
        .owner = THIS_MODULE,
        .of_match_table = irda_of_match
    },
    .probe = irda_probe,
    .remove = irda_remove
};


static int irda_init(void){
    platform_driver_register(&irda_driver);
    printk(KERN_EMERG "irda_init()\n");
    return 0;
}

static void irda_exit(void){
    platform_driver_unregister(&irda_driver);
    printk(KERN_EMERG "irda_exit()\n");
}


module_init(irda_init);
module_exit(irda_exit);
MODULE_LICENSE("GPL");
