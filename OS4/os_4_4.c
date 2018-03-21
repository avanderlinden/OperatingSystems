#include <linux/gpio.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/sched.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/mutex.h>

#define LED1 4
#define LED2 17
#define LED3 27
#define LED4 22

#define HIGH 1
#define LOW 0

#define PERIOD_NS 10000000

int init_module(void);
void cleanup_module(void);
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

static struct file_operations fops = {
    .read = device_read,
    .write = device_write,
    .open = device_open,
    .release = device_release
};

static int major;
struct class* pwm_class;
struct device* device;

int DevCounter;

struct mutex dev_mutex;

/*
 * Struct gpio_config_leds is a configuration for the
 * GPIOs that are connected to the TU Delft LED module.
 */
static struct gpio gpio_config_leds[] = {
        { LED1, GPIOF_OUT_INIT_LOW,  "LED 1" },
        { LED2, GPIOF_OUT_INIT_LOW,  "LED 2" },
        { LED3, GPIOF_OUT_INIT_LOW,  "LED 3" },
        { LED4, GPIOF_OUT_INIT_LOW,  "LED 4" },
    };

int led_flag;
int duty_cycle;
int new_duty_cycle;

static ktime_t on_time;
static ktime_t off_time;

static struct hrtimer hr_timer;
static enum hrtimer_restart led_pwm(struct hrtimer *);

int init_module(void)
{
    MODULE_LICENSE("GPL");
    MODULE_AUTHOR("Alex van der Linden");
    MODULE_DESCRIPTION("Create a pwm device");
    MODULE_VERSION("0.1");

    int status;

    mutex_init(&dev_mutex);

    // init char device
    printk(KERN_INFO "Create kernel device /dev/pwm\n");

    major = register_chrdev(0, "pwm", &fops);
    DevCounter = 0;

    if (major < 0) {
      printk(KERN_ALERT "Registering device failed with %d\n", major);
      return 1;
    }

    pwm_class = class_create (THIS_MODULE,"pwm_class");
    device = device_create(pwm_class,NULL,MKDEV(major, 0),NULL,"pwm");

    // init GPIOs
    status = gpio_request_array(gpio_config_leds, ARRAY_SIZE(gpio_config_leds));
    if(status){
        return 1;
    }

    gpio_set_value(LED1, HIGH);
    gpio_set_value(LED2, HIGH);
    gpio_set_value(LED3, HIGH);
    gpio_set_value(LED4, HIGH);

    duty_cycle = 50;
    new_duty_cycle = 60;

    // init timer
    on_time = ktime_set(0,PERIOD_NS/2);
    off_time = ktime_set(0,PERIOD_NS/2);

    hrtimer_init(&hr_timer, CLOCK_REALTIME, HRTIMER_MODE_REL);
    hr_timer.function = led_pwm;

    led_flag = HIGH;
    hrtimer_start(&hr_timer, on_time, HRTIMER_MODE_REL);



    return 0;
}

void cleanup_module(void)
{
    printk(KERN_INFO "Lights off!\n");

    hrtimer_cancel(&hr_timer);

    gpio_set_value(LED1, LOW);
    gpio_set_value(LED2, LOW);
    gpio_set_value(LED3, LOW);
    gpio_set_value(LED4, LOW);

    gpio_free_array(gpio_config_leds, ARRAY_SIZE(gpio_config_leds));

    printk(KERN_INFO "Cleanup kernel device /dev/pwm\n");

    device_destroy(pwm_class, MKDEV(major, 0));
    class_unregister(pwm_class);
    class_destroy(pwm_class);

    unregister_chrdev(major, "pwm");
}


/*
 * Called when a process tries to open the device file.
 */
static int device_open(struct inode *inode, struct file *file)
{
    mutex_lock(&dev_mutex);

    if (DevCounter){
        return -EBUSY;
    }

    DevCounter++;
    try_module_get(THIS_MODULE);

    return 0;
}

/*
 * Called when a process closes the device file.
 */
static int device_release(struct inode *inode, struct file *file)
{
    DevCounter--;
    module_put(THIS_MODULE);

    mutex_unlock(&dev_mutex);

    return 0;
}



/*
 * Called when a process, which already opened the dev file, attempts to
 * read from it.
 */
static ssize_t device_read(struct file *filp,   /* see include/linux/fs.h   */
               char *buffer,    /* buffer to fill with data */
               size_t length,   /* length of the buffer     */
               loff_t * offset)
{

    char msg[80] = "You are reading me \0";

    int bytes_read = 0;
    char c = 'x';

    while (length && c != '\0') {

        c = msg[bytes_read];

        put_user(c, buffer++);

        length--;
        bytes_read++;
    }

    return bytes_read; // return the number of bytes send to caller.
}

/*
 * Called when a process writes to dev file: echo "hi" > /dev/hello
 */
static ssize_t device_write(struct file *filp,
                            const char *buff,
                            size_t len,
                            loff_t * off)
{
    int data = -1;
    char msg[len];

    strncpy(msg, buff, len);
    sscanf(msg, "%d\n", &data);

    if((data >= 0) && (data <= 100)){
        printk(KERN_INFO "New Duty Cycle: %d\n", data);
        new_duty_cycle = data;
        return len;
    }
    else {
        printk(KERN_INFO "Illegal input: %s\n", buff);
        return -EINVAL;
    }
}

static enum hrtimer_restart led_pwm(struct hrtimer * hrtim){

    int on_time_ns;
    int off_time_ns;

    if(!(duty_cycle == new_duty_cycle)){
        duty_cycle = new_duty_cycle;

        on_time_ns = (duty_cycle * PERIOD_NS) / 100;
        off_time_ns = PERIOD_NS - on_time_ns;

        on_time = ktime_set(0, on_time_ns);
        off_time = ktime_set(0, off_time_ns);
    }

    if(duty_cycle == 100){
        gpio_set_value(LED1, HIGH);
        gpio_set_value(LED2, HIGH);
        gpio_set_value(LED3, HIGH);
        gpio_set_value(LED4, HIGH);
        led_flag = HIGH;
        hrtimer_forward_now(&hr_timer, ktime_set(0,10000000));
    }
    else if(duty_cycle == 0){
        gpio_set_value(LED1, LOW);
        gpio_set_value(LED2, LOW);
        gpio_set_value(LED3, LOW);
        gpio_set_value(LED4, LOW);
        led_flag = LOW;
        hrtimer_forward_now(&hr_timer, ktime_set(0,10000000));
    }
    else{
        if(led_flag){
                gpio_set_value(LED1, LOW);
                gpio_set_value(LED2, LOW);
                gpio_set_value(LED3, LOW);
                gpio_set_value(LED4, LOW);
                led_flag = LOW;
                hrtimer_forward_now(&hr_timer, off_time);
            }
            else{
                gpio_set_value(LED1, HIGH);
                gpio_set_value(LED2, HIGH);
                gpio_set_value(LED3, HIGH);
                gpio_set_value(LED4, HIGH);
                led_flag = HIGH;
                hrtimer_forward_now(&hr_timer, on_time);
            }
    }
    return HRTIMER_RESTART;

}
