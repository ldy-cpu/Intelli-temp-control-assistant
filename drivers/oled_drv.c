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

#include<linux/spi/spi.h>

#define OLED_CMD 0
#define OLED_DATA 1

// int gpio;
static struct gpio_desc *gpio;
int major;
dev_t devno;
struct device_node *node;
// static int major;
static struct class *oled_class;
static struct device *oled_device;
static struct spi_device *spi_oled;
int r;

struct cdev cdev = {
    .owner = THIS_MODULE
};

static long oled_ioctl(struct file *filp,unsigned int cmd,unsigned long arg){

    struct spi_message msg;
    struct spi_transfer xfer[3];

    spi_message_init(&msg);
    spi_message_add_tail(&xfer[0],&msg);
    printk(KERN_EMERG "oled_ioctl\n");
    return 0;
}

static struct file_operations oled_fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = oled_ioctl,
};


static void oled_write_cmd_data(unsigned char uc_data,unsigned char uc_cmd){
    if(uc_cmd == 0){
        gpiod_set_value(gpio,0);
        // printk(KERN_EMERG "0 gpio = %d\n",gpiod_get_value(gpio));
    }
    else{
        gpiod_set_value(gpio,1);
        // printk(KERN_EMERG "1 gpio = %d\n",gpiod_get_value(gpio));
    }

    spi_write(spi_oled,&uc_data,1);

}

void oled_set_pos(int x, int y){
    oled_write_cmd_data(0xb0+y,OLED_CMD);
    
    // oled_write_cmd_data((x&0x0f),OLED_CMD);
    // oled_write_cmd_data(((x&0xf0)>>4)|0x10,OLED_CMD);

    oled_write_cmd_data((x&0x0f),OLED_CMD); 
	oled_write_cmd_data(((x&0xf0)>>4)|0x10,OLED_CMD);
}

static void oled_clear(void){
    unsigned char x, y;
    for(y = 0;y<8;y++){
        oled_set_pos(0,y);
        // for(i = 0;i<64;i++){
        //     oled_write_cmd_data(0,0xff);//清屏
        // }
        // for(i = 64;i<128;i++){
        //     oled_write_cmd_data(0xff,0xff);//清屏
        // }
        for(x = 0;x<128;x++){
            oled_write_cmd_data(0xff,OLED_DATA);//清屏
        }
    }
}



static void oled_show_init(void){
    // gpio_direction_output(gpio,0);
    


    // oled_write_cmd_data(0xAE,0); // 关闭oled面板
    // oled_write_cmd_data(0x00,0); // 设置低列地址
    // oled_write_cmd_data(0x10,0); // 设置高列地址
    // oled_write_cmd_data(0x40,0); // 设置起始行地址
    // oled_write_cmd_data(0xB0,0); // 设置页地址
    // oled_write_cmd_data(0x81,0); 
    // oled_write_cmd_data(0x66,0); //128级对比度设置

    // oled_write_cmd_data(0xA1,0); // 设置段重定义
    // oled_write_cmd_data(0xA6,0); // 设置正常显示
    // oled_write_cmd_data(0xA8,0); // 设置多路复用率
    // oled_write_cmd_data(0x3F,0); // 设置多路复用率

    // oled_write_cmd_data(0xC8,0); // 设置COM扫描方向
    // oled_write_cmd_data(0xD3,0); // 设置显示偏移
    // oled_write_cmd_data(0x00,0); // 设置显示偏移
    // oled_write_cmd_data(0xD5,0); // 设置显示时钟分频
    // oled_write_cmd_data(0x80,0); // 设置显示时钟分频
    // oled_write_cmd_data(0xD9,0); // 设置预充电周期
    // oled_write_cmd_data(0x1F,0); // 设置预充电周期
    // oled_write_cmd_data(0xDA,0); // 设置COM引脚硬件配置
    // oled_write_cmd_data(0x12,0); // 设置COM引脚硬件配置
    // oled_write_cmd_data(0xDB,0); // 设置VCOMH
    // oled_write_cmd_data(0x30,0); // 设置VCOMH
    // oled_write_cmd_data(0x8D,0); // 设置电荷泵
    // oled_write_cmd_data(0x14,0); // 设置电荷泵
    // oled_write_cmd_data(0xAF,0); // 打开oled面板



    oled_write_cmd_data(0xae,OLED_CMD);//关闭显示

	oled_write_cmd_data(0x00,OLED_CMD);//设置 lower column address
	oled_write_cmd_data(0x10,OLED_CMD);//设置 higher column address

	oled_write_cmd_data(0x40,OLED_CMD);//设置 display start line

	oled_write_cmd_data(0xB0,OLED_CMD);//设置page address

	oled_write_cmd_data(0x81,OLED_CMD);// contract control
	oled_write_cmd_data(0x66,OLED_CMD);//128

	oled_write_cmd_data(0xa1,OLED_CMD);//设置 segment remap

	oled_write_cmd_data(0xa6,OLED_CMD);//normal /reverse

	oled_write_cmd_data(0xa8,OLED_CMD);//multiple ratio
	oled_write_cmd_data(0x3f,OLED_CMD);//duty = 1/64

	oled_write_cmd_data(0xc8,OLED_CMD);//com scan direction

	oled_write_cmd_data(0xd3,OLED_CMD);//set displat offset
	oled_write_cmd_data(0x00,OLED_CMD);//

	oled_write_cmd_data(0xd5,OLED_CMD);//set osc division
	oled_write_cmd_data(0x80,OLED_CMD);//

	oled_write_cmd_data(0xd9,OLED_CMD);//ser pre-charge period
	oled_write_cmd_data(0x1f,OLED_CMD);//

	oled_write_cmd_data(0xda,OLED_CMD);//set com pins
	oled_write_cmd_data(0x12,OLED_CMD);//

	oled_write_cmd_data(0xdb,OLED_CMD);//set vcomh
	oled_write_cmd_data(0x30,OLED_CMD);//

	oled_write_cmd_data(0x8d,OLED_CMD);//set charge pump disable 
	oled_write_cmd_data(0x14,OLED_CMD);//

	oled_write_cmd_data(0xaf,OLED_CMD);//set dispkay on


}

static int oled_probe(struct spi_device *spi){


    // int ret;
    spi_oled = spi;
    // node = spi->dev.of_node;
    
    // if(node == NULL){
    //     printk(KERN_EMERG "node == NULL\n");
    //     return -1;
    // }

    // ret = alloc_chrdev_region(&devno,0,1,"oled");
    // if(ret < 0){
    //     printk(KERN_EMERG "alloc_chrdev_region() failed\n");
    //     return -1;
    // }

    // cdev_init(&cdev,&oled_fops);
    // cdev_add(&cdev,devno,1);

    // oled_class = class_create(THIS_MODULE,"oled_class");
    // oled_device = device_create(oled_class,NULL,devno,NULL,"oled");


    major = register_chrdev(0,"oled",&oled_fops);
    oled_class = class_create(THIS_MODULE,"oled_class");
    oled_device = device_create(oled_class,NULL,MKDEV(major,0),NULL,"oled");


    // gpio = of_get_named_gpio(node,"dc-gpios",1);
    // gpio_direction_output(spi->cs_gpio,1);
    gpio_set_value(spi->cs_gpio,0);
    printk(KERN_EMERG "spi->cs_gpio = %d\n",spi->cs_gpio);
    
    gpio = gpiod_get(&spi->dev,"dc",1);
    if(gpio == NULL){
        printk(KERN_EMERG "gpio == NULL\n");
        return -1;
    }
    // gpiod_direction_output(gpio,0);
    gpiod_direction_output(gpio,1);
    oled_show_init();
    oled_clear();
    printk(KERN_EMERG "spi probe\n");
    return 0;
}

static int oled_remove(struct spi_device *spi){

    // device_destroy(oled_class,devno);
    // class_destroy(oled_class);
    // cdev_del(&cdev);
    // unregister_chrdev_region(devno,1);

    device_destroy(oled_class,MKDEV(major,0));
    class_destroy(oled_class);
    unregister_chrdev(major,"oled");


    gpiod_put(gpio);
    printk(KERN_EMERG "spi remove\n");
    return 0;
}



static const struct of_device_id oled_id_table[] = {
    {.compatible = "oled"},
    {}
};

static struct spi_driver oled_driver = {
    .driver = {
        .name = "oled",
        .of_match_table = oled_id_table,
    },
    // .id_table = oled_id_table,
    .probe = oled_probe,
    .remove = oled_remove,
};


int oled_init(void){
    return spi_register_driver(&oled_driver);
}

void oled_exit(void){
    spi_unregister_driver(&oled_driver);
}

module_init(oled_init);
module_exit(oled_exit);

MODULE_LICENSE("GPL");