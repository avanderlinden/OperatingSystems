
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <asm/uaccess.h>


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
struct class* klog_class;
struct device* device;

int DevCounter;



int init_module(void)
{
    MODULE_LICENSE("GPL");
    MODULE_AUTHOR("Alex van der Linden");
    MODULE_DESCRIPTION("Create a kernel log device");
    MODULE_VERSION("0.1");

    printk(KERN_INFO "Create kernel log device /dev/klog");

    major = register_chrdev(0, "klog", &fops);
    DevCounter = 0;

    if (major < 0) {
      printk(KERN_ALERT "Registering device failed with %d\n", major);
      return 1;
    }


    klog_class = class_create (THIS_MODULE,"klog_class");
    device = device_create(klog_class,NULL,MKDEV(major, 0),NULL,"klog");

    return 0;
}

void cleanup_module(void)
{
    printk(KERN_INFO "Cleanup kernel log device /dev/klog");

    device_destroy(klog_class, MKDEV(major, 0));
    class_unregister(klog_class);
    class_destroy(klog_class);

    unregister_chrdev(major, "klog");
}


/*
 * Called when a process tries to open the device file.
 */
static int device_open(struct inode *inode, struct file *file)
{
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
    printk(KERN_INFO "%s\n", buff);

    return len;
}


