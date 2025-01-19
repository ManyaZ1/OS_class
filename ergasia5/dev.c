#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/wait.h>

#define MY_MAJOR 42
#define MY_MINOR 0
#define NUM_MINORS 2
#define MODULE_NAME "MyDevice"
#define MESSAGE "hello\n"
#define BUFFER_SIZE 4096

MODULE_DESCRIPTION("Character Device Driver");
MODULE_AUTHOR("Me");
MODULE_LICENSE("GPL");

struct my_device_data {
    struct cdev cdev; // Χειριστής character device
    char buffer[BUFFER_SIZE]; // Buffer αποθήκευσης δεδομένων
    size_t size; // Μέγεθος buffer
    atomic_t access; // Διαχείριση πρόσβασης
};

struct my_device_data devs[NUM_MINORS];

static int my_cdev_open(struct inode *inode, struct file *file) {
    struct my_device_data *data;

    printk(KERN_INFO "Device opened.\n");

    data = container_of(inode->i_cdev, struct my_device_data, cdev);
    file->private_data = data;

    if (atomic_cmpxchg(&data->access, 0, 1) != 0) {
        return -EBUSY;
    }

    return 0;
}

static int my_cdev_release(struct inode *inode, struct file *file) {
    struct my_device_data *data = (struct my_device_data *)file->private_data;

    printk(KERN_INFO "Device closed.\n");

    atomic_set(&data->access, 0);
    return 0;
}

static ssize_t my_cdev_read(struct file *file, char __user *user_buffer, size_t size, loff_t *offset) {
    struct my_device_data *data = (struct my_device_data *)file->private_data;
    size_t to_read;

    to_read = (size > data->size - *offset) ? (data->size - *offset) : size;

    if (copy_to_user(user_buffer, data->buffer + *offset, to_read)) {
        return -EFAULT;
    }

    *offset += to_read;
    printk(KERN_INFO "Read %zu bytes from device.\n", to_read);

    return to_read;
}

static ssize_t my_cdev_write(struct file *file, const char __user *user_buffer, size_t size, loff_t *offset) {
    struct my_device_data *data = (struct my_device_data *)file->private_data;

    size = (*offset + size > BUFFER_SIZE) ? (BUFFER_SIZE - *offset) : size;

    if (copy_from_user(data->buffer + *offset, user_buffer, size)) {
        return -EFAULT;
    }

    *offset += size;
    data->size = *offset;
    printk(KERN_INFO "Wrote %zu bytes to device.\n", size);

    return size;
}

static const struct file_operations my_fops = {
    .owner = THIS_MODULE,
    .open = my_cdev_open,
    .release = my_cdev_release,
    .read = my_cdev_read,
    .write = my_cdev_write,
};

static int my_init(void) {
    int err, i;

    err = register_chrdev_region(MKDEV(MY_MAJOR, MY_MINOR), NUM_MINORS, MODULE_NAME);
    if (err != 0) {
        pr_info("Failed to register character device region.\n");
        return err;
    }

    for (i = 0; i < NUM_MINORS; i++) {
        memcpy(devs[i].buffer, MESSAGE, sizeof(MESSAGE));
        devs[i].size = sizeof(MESSAGE);
        atomic_set(&devs[i].access, 0);

        cdev_init(&devs[i].cdev, &my_fops);
        devs[i].cdev.owner = THIS_MODULE;
        err = cdev_add(&devs[i].cdev, MKDEV(MY_MAJOR, MY_MINOR + i), 1);
        if (err) {
            pr_info("Failed to add cdev.\n");
            return err;
        }
    }

    printk(KERN_INFO "Module loaded.\n");
    return 0;
}

static void my_exit(void) {
    int i;

    for (i = 0; i < NUM_MINORS; i++) {
        cdev_del(&devs[i].cdev);
    }

    unregister_chrdev_region(MKDEV(MY_MAJOR, MY_MINOR), NUM_MINORS);
    printk(KERN_INFO "Module unloaded.\n");
}

module_init(my_init);
module_exit(my_exit);
