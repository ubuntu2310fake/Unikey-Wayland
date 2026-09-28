#include <linux/module.h>
#include <linux/input.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/miscdevice.h>
#include <linux/kfifo.h>
#include <linux/wait.h>
#include <linux/poll.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

MODULE_AUTHOR("Antigravity");
MODULE_DESCRIPTION("Unikey Wayland Ring 0 Grabber & Ring 3 Injector");
MODULE_LICENSE("GPL");

static struct input_dev *ukw_vdev;
static struct input_dev *phys_dev = NULL;

struct ukw_device {
    struct input_handle handle;
};

struct ukw_event {
    unsigned short type;
    unsigned short code;
    int value;
};

DECLARE_KFIFO(ukw_fifo, struct ukw_event, 1024);
static DECLARE_WAIT_QUEUE_HEAD(ukw_waitq);
static DEFINE_SPINLOCK(ukw_lock);

static ssize_t ukw_read(struct file *file, char __user *ubuf, size_t count, loff_t *ppos) {
    int copied = 0;
    int ret;

    if (kfifo_is_empty(&ukw_fifo)) {
        if (file->f_flags & O_NONBLOCK) return -EAGAIN;
        if (wait_event_interruptible(ukw_waitq, !kfifo_is_empty(&ukw_fifo)))
            return -ERESTARTSYS;
    }
    
    ret = kfifo_to_user(&ukw_fifo, ubuf, count, &copied);
    if (ret) return -EFAULT;
        
    return copied;
}

static ssize_t ukw_write(struct file *file, const char __user *ubuf, size_t count, loff_t *ppos) {
    struct ukw_event ev;
    size_t i;
    for (i = 0; i < count / sizeof(ev); i++) {
        if (copy_from_user(&ev, ubuf + i * sizeof(ev), sizeof(ev))) return -EFAULT;
        
        input_event(ukw_vdev, ev.type, ev.code, ev.value);
        
        if (ev.type == EV_SYN) {
            udelay(20);
        }
    }
    return count;
}

static __poll_t ukw_poll(struct file *file, poll_table *wait) {
    poll_wait(file, &ukw_waitq, wait);
    if (!kfifo_is_empty(&ukw_fifo)) return EPOLLIN | EPOLLRDNORM;
    return 0;
}

static int ukw_release(struct inode *inode, struct file *file) {
    int i;
    for (i = 0; i < KEY_MAX; i++) {
        if (test_bit(i, ukw_vdev->key)) {
            input_event(ukw_vdev, EV_KEY, i, 0);
        }
    }
    input_sync(ukw_vdev);
    kfifo_reset(&ukw_fifo);
    return 0;
}

static const struct file_operations ukw_fops = {
    .owner = THIS_MODULE,
    .read = ukw_read,
    .write = ukw_write,
    .release = ukw_release,
    .poll = ukw_poll,
};

static struct miscdevice ukw_misc = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "ukw",
    .fops = &ukw_fops,
    .mode = 0666,
};

static void ukw_event_handler(struct input_handle *handle, unsigned int type, unsigned int code, int value) {
    struct ukw_event ev;
    ev.type = type;
    ev.code = code;
    ev.value = value;
    
    kfifo_in_spinlocked(&ukw_fifo, &ev, 1, &ukw_lock);
    wake_up_interruptible(&ukw_waitq);
}

static int ukw_connect(struct input_handler *handler, struct input_dev *dev, const struct input_device_id *id) {
    struct ukw_device *ukw_dev;
    int error;

    if (dev == ukw_vdev) return -ENODEV;
    if (test_bit(BTN_TOUCH, dev->keybit)) return -ENODEV;
    if (!test_bit(KEY_A, dev->keybit) || !test_bit(KEY_SPACE, dev->keybit)) return -ENODEV;

    ukw_dev = kzalloc(sizeof(struct ukw_device), GFP_KERNEL);
    if (!ukw_dev) return -ENOMEM;

    ukw_dev->handle.dev = dev;
    ukw_dev->handle.handler = handler;
    ukw_dev->handle.name = "ukw_handle";
    ukw_dev->handle.private = ukw_dev;

    error = input_register_handle(&ukw_dev->handle);
    if (error) goto err_free;

    error = input_open_device(&ukw_dev->handle);
    if (error) goto err_unregister;

    error = input_grab_device(&ukw_dev->handle);
    if (!error) {
        phys_dev = dev; // Lưu lại con trỏ thiết bị vật lý để bật/tắt đèn LED
        pr_info("ukw: Keyboard grabbed: %s\n", dev->name);
    }

    return 0;

err_unregister:
    input_unregister_handle(&ukw_dev->handle);
err_free:
    kfree(ukw_dev);
    return error;
}

static void ukw_disconnect(struct input_handle *handle) {
    struct ukw_device *ukw_dev = handle->private;
    if (phys_dev == handle->dev) phys_dev = NULL;
    input_release_device(handle);
    input_close_device(handle);
    input_unregister_handle(handle);
    kfree(ukw_dev);
}

static const struct input_device_id ukw_ids[] = {
    { .flags = INPUT_DEVICE_ID_MATCH_EVBIT, .evbit = { BIT_MASK(EV_KEY) } },
    { },
};
MODULE_DEVICE_TABLE(input, ukw_ids);

static struct input_handler ukw_handler = {
    .event = ukw_event_handler,
    .connect = ukw_connect,
    .disconnect = ukw_disconnect,
    .name = "ukw_driver",
    .id_table = ukw_ids,
};

static int ukw_vdev_event(struct input_dev *dev, unsigned int type, unsigned int code, int value) {
    if (type == EV_LED && phys_dev && phys_dev->event) {
        // Forward tín hiệu bật/tắt đèn LED từ Wayland xuống phần cứng vật lý
        phys_dev->event(phys_dev, type, code, value);
    }
    return 0;
}

static int __init ukw_init(void) {
    int i, error;

    INIT_KFIFO(ukw_fifo);

    error = misc_register(&ukw_misc);
    if (error) {
        pr_err("ukw: failed to register misc device\n");
        return error;
    }

    ukw_vdev = input_allocate_device();
    if (!ukw_vdev) {
        misc_deregister(&ukw_misc);
        return -ENOMEM;
    }

    ukw_vdev->name = "Unikey Wayland Virtual Keyboard";
    ukw_vdev->id.bustype = BUS_VIRTUAL;
    ukw_vdev->event = ukw_vdev_event; // Hook LED events
    
    ukw_vdev->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_SYN) | BIT_MASK(EV_REL) | BIT_MASK(EV_LED);
    
    for (i = 0; i < KEY_MAX; i++) set_bit(i, ukw_vdev->keybit);
    for (i = 0; i < REL_MAX; i++) set_bit(i, ukw_vdev->relbit);
    set_bit(LED_CAPSL, ukw_vdev->ledbit);
    set_bit(LED_NUML, ukw_vdev->ledbit);
    set_bit(LED_SCROLLL, ukw_vdev->ledbit);

    error = input_register_device(ukw_vdev);
    if (error) {
        input_free_device(ukw_vdev);
        misc_deregister(&ukw_misc);
        return error;
    }

    error = input_register_handler(&ukw_handler);
    if (error) {
        input_unregister_device(ukw_vdev);
        misc_deregister(&ukw_misc);
        return error;
    }

    pr_info("ukw: Hybrid Ring 0/3 Driver loaded successfully (/dev/ukw ready)\n");
    return 0;
}

static void __exit ukw_exit(void) {
    input_unregister_handler(&ukw_handler);
    input_unregister_device(ukw_vdev);
    misc_deregister(&ukw_misc);
    pr_info("ukw: Hybrid Driver unloaded\n");
}

module_init(ukw_init);
module_exit(ukw_exit);
