#include <linux/module.h>  
#include <linux/printk.h>  
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/pwm.h>

#define DEVICE_NAME	"my_pwm_device"
#define DRIVER_NAME  	"my_pwm_0_driver"
#define DRIVER_CLASS  	"my_pwm_class"
#define PWM_ON_TIME_NS	500000000UL
#define PERIOD		1000000000UL

/**
 * @brief Device-related variables
 *
 * These variables are used for managing the PWM device and its associated class.
 */
static dev_t my_pwm_dev;
static struct class *my_pwm_class;
static struct cdev my_pwm_cdev;

/**
 * @brief PWM-related variables
 *
 * These variable are used for managing the PWM device.
 */
struct pwm_device *ptr_pwm = NULL;


/**
 * @brief Writes user-input percentage to configure PWM.
 *
 * This function is called when a user-space program writes a percentage value to the PWM driver.
 * It converts the user input to a long integer, validates its range (0-100), and configures the PWM
 * based on the provided percentage.
 *
 * @return The number of bytes successfully written or a negative error code on failure.
 */
static ssize_t driver_write(struct file *File, const char __user *buffer, size_t count, loff_t *offs) {
	int size;
	long percent;
        char userInput[9];
	
	size = min(count,sizeof(userInput));

	if(copy_from_user(&userInput, buffer, size))
		return -EFAULT;

	userInput[sizeof(userInput)- 1] = '\0';
	if(kstrtol(userInput, 10, &percent))
		return -EINVAL;

	printk("Duty Cycle: %ld\n", percent);
	if(percent < 0 || percent > 100) {
		pr_err("Invalide Input: %ld\n", percent);
		return -EINVAL;
	}
	else 
		pwm_config(ptr_pwm, 10000000 * percent , PERIOD);

	*offs += size;
	return size;
}
static int driver_open(struct inode *device_file, struct file *instance) {
	pr_info("Device opened: %s\n", DEVICE_NAME);
	return 0;
}

static int driver_close(struct inode *device_file, struct file *instance) {
	pr_info("Device closed: %s\n", DEVICE_NAME);
	return 0;
}

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = driver_open,
	.release = driver_close,
	.write = driver_write
};

/**
 * @brief Initializes the PWM module.
 *
 * This function is called during module initialization. It allocates a device number,
 * creates a device class, and initializes the device file. It also configures and enables
 * the PWM device.
 *
 * @return 0 on success, or a negative error code on failure.
 */
static int __init initmodule(void) { 

	// Allocate a device number for the PWM driver
	if( alloc_chrdev_region(&my_pwm_dev, 0 , 1, DRIVER_NAME) < 0) {
		pr_err("Failed to allocate device number!\n");
		return -1;
	}

	// Create a device class for the PWM driver
	if((my_pwm_class = class_create(THIS_MODULE, DRIVER_CLASS)) == NULL) {
		pr_err("Failed to create the character device class!\n");
		goto ClassError;
	}

	// Create the device file associated with the PWM driver
	if(device_create(my_pwm_class, NULL, my_pwm_dev, NULL, DRIVER_NAME) == NULL) {
		pr_err("Failed to create the character device file!\n");
		goto FileError;
	}

	// Initialize the character device structure and add it to the systen
	cdev_init(&my_pwm_cdev, &fops);
	if(cdev_add(&my_pwm_cdev, my_pwm_dev, 1) == -1) {
		pr_err("Failed to register the character device to /dev!\n");
		goto AddError;
	}

	// Request the PWM device and configure it
	ptr_pwm = pwm_request(0, "pwm_0");
	if(ptr_pwm == NULL) {
		pr_err("Failed to request PWM device!\n");
		goto AddError;
	}

	// Configure the PWM parameters and enable it
	pwm_config(ptr_pwm, PWM_ON_TIME_NS, PERIOD);
	pwm_enable(ptr_pwm);
	pr_info("PWM device initialized successfully. Label: %d\n", ptr_pwm->pwm);

	return 0;
AddError:
	device_destroy(my_pwm_class, my_pwm_dev);
FileError:
	class_destroy(my_pwm_class);
ClassError:
	unregister_chrdev(my_pwm_dev, DRIVER_NAME);
	return -1;

} 

static void __exit exitmodule(void) {
	pwm_disable(ptr_pwm);
	pwm_free(ptr_pwm);
	cdev_del(&my_pwm_cdev);
	device_destroy(my_pwm_class, my_pwm_dev);
	class_destroy(my_pwm_class);
	unregister_chrdev(my_pwm_dev, DRIVER_NAME);
}

module_init(initmodule);
module_exit(exitmodule);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Timo Rosenberg");
MODULE_DESCRIPTION("A Simple PWM LKM");
