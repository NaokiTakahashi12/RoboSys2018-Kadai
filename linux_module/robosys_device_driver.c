#include <linux/module.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/io.h>

MODULE_AUTHOR("Naoki Takahashi");
MODULE_DESCRIPTION("Driver for RoboSys2018");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");

static dev_t device;
static struct class *class_ptr = NULL;
static struct cdev character_device;
static volatile u32 *gpio_base = NULL;

static unsigned int send_gpios[] = {24, 23, 22, 27, 17, 25, 18, 4};
static unsigned int listen_gpios[] = {21, 20, 16, 26, 19, 13, 6, 5};

static ssize_t character_module_write(struct file *file_ptr, const char *buffer, size_t count, loff_t *pos) {
	static int gpio_transition = 1;
	const int clear = 10;
	const int value = 7;
	char c;

	printk(KERN_INFO "Write is called. ");

	if(copy_from_user(&c, buffer, sizeof(char))) {
		return -EFAULT;
	}

	if(c == '0') {
		int i;
		for(i = 0; i < 8; i ++) {
			gpio_base[clear] = 1 << send_gpios[i];
		}
		gpio_transition = 1;
		printk(KERN_INFO "Led is down. ");
	}
	else if(c == '1') {
		int i;
		if(gpio_transition > 8) {
			gpio_transition = 1;
			for(i = 0; i < 8; i ++) {
				gpio_base[clear] = 1 << send_gpios[i];
			}
		}
		for(i = 0; i < gpio_transition; i ++) {
			gpio_base[value] = 1 << send_gpios[i];
		}
		gpio_transition ++;
		printk(KERN_INFO "Led is up. ");
	}
	else if(c != '\n') {
		int i = 0;
		for(i = 0; i < 8; i ++) {
			gpio_base[clear] = 1 << send_gpios[i];
		}
		i = 0;
		if(c & 1) {
			gpio_base[value] = 1 << send_gpios[i];
		}
		for(i = 1; i < 7; i ++) {
			int number_of_binary = 2;
			int j;

			for(j = 1; j < i; j ++) {
				number_of_binary *= 2;
			}
			if(c & number_of_binary) {
				gpio_base[value] = 1 << send_gpios[i];
			}
		}
		printk(KERN_INFO "Led is string up. : %c", c);
	}

	return 1;
}

static ssize_t character_module_read(struct file *file_ptr, char *buffer, size_t count, loff_t *pos) {
	const int status = 13;
	int size = 0;
	int i;
	char catch_string[] = {0x0, 0x0};

	for(i = 0; i < 8; i ++) {
		if(gpio_base[status] & (1 << listen_gpios[i])) {
			catch_string[0] = 1 << (7 - i);
		}
	}

	if('!' > catch_string[0] || catch_string[0] > '~') {
		return 0;
	}

	catch_string[1] = '\n';

	if(copy_to_user(buffer + size, (const char *)catch_string, sizeof(catch_string))) {
		printk(KERN_INFO "copy_to_user() failed.\n");
		return -EFAULT;
	}

	size += sizeof(catch_string);

	return size;
}

static struct file_operations led_file_operations = {
	.owner = THIS_MODULE,
	.write = character_module_write,
	.read = character_module_read
};

void gpio_output(int number_of_bcm) {
	u32 led, index, shift, mask;

	gpio_base = ioremap_nocache(0x3F200000, 0xA0);
	led = (u32)number_of_bcm;
	index = (u32)(led / 10);
	shift = (led % 10) * 3;
	mask = ~(0x7 << shift);
	gpio_base[index] = (gpio_base[index] & mask) | (0x001 << shift);
}

void gpio_input(int number_of_bcm) {
	u32 led, index, shift, mask;

	gpio_base = ioremap_nocache(0x3F200000, 0xA0);
	led = (u32)number_of_bcm;
	index = (u32)(led / 10);
	shift = (led % 10) * 3;
	mask = ~(0x7 << shift);
	gpio_base[index] = (0x0 << shift);
	gpio_base[index] = gpio_base[index] | (gpio_base[index] & mask);
}

static int __init init_mod(void) {
	int return_region_value;
	int return_character_device_add;
	int i;

	for(i = 0; i < 8; i ++) {
		gpio_input(listen_gpios[i]);
	}

	for(i = 0; i < 8; i ++) {
		gpio_output(send_gpios[i]);
	}

	return_region_value = alloc_chrdev_region(&device, 0, 1, "robosys_device");
	if(return_region_value < 0) {
		printk(KERN_ERR "alloc_chrdev_region failed. major number:%d minor number:%d\n", MAJOR(device), MINOR(device));
		return return_region_value;
	}

	cdev_init(&character_device, &led_file_operations);
	return_character_device_add = cdev_add(&character_device, device, 1);
	if(return_character_device_add < 0) {
		printk(KERN_ERR "cdev_add failed. major numebr:%d minor number:%d\n", MAJOR(device), MINOR(device));
		return return_character_device_add;
	}

	class_ptr = class_create(THIS_MODULE, "robosys_device");
	if(IS_ERR(class_ptr)) {
		printk(KERN_ERR "class_create failed.\n");
		return PTR_ERR(class_ptr);
	}

	device_create(class_ptr, NULL, device, NULL, "robosys_device%d", MINOR(device));
	printk(KERN_INFO "%s is loaded. major number %d.\n", __FILE__, MAJOR(device));

	return 0;
}

static void  __exit cleanup_mod(void) {
	device_destroy(class_ptr, device);
	class_destroy(class_ptr);
	cdev_del(&character_device);
	unregister_chrdev_region(device, 1);

	printk(KERN_INFO "%s is purge. major number is %d\n", __FILE__, MAJOR(device));
}

module_init(init_mod);
module_exit(cleanup_mod);

