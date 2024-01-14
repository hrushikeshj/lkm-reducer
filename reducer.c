#include<linux/kernel.h>
#include<linux/module.h>
#include<linux/fs.h>
#include<asm/uaccess.h>

#define DEVICE_NAME "reducer"
#define BUF_LEN 80
#define RING_BUF_LEN 512
#define DEBUG 1

/* Meta Information */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Hrushi");
MODULE_DESCRIPTION("A char device which sums up the numbers written to it");

static int device_open(struct inode *inode, struct file *file);
static int device_release(struct inode *inode, struct file *file);
static ssize_t device_read(struct file *file, char __user *buffer, size_t length, loff_t *offset);
static ssize_t
device_write(struct file *filp, const char *buff, size_t len, loff_t * off);

static int Major;
static int Device_open = 0;
static char buf[BUF_LEN];
static char *msg_Ptr;

static char ring_buff[RING_BUF_LEN];
static int head, tail;
// tail - next place were data is written to

static void init_ring_buf(void){
    head = 0;
    tail = 0;
}

static void print_ring_buff(void){
    int i;
    printk(KERN_INFO "");
    for(i=0; i<RING_BUF_LEN; i++){
        printk(KERN_CONT "%c,", ring_buff[i]);
    }
    printk(KERN_INFO "%d, %d", head, tail);
}


static int ring_empty(void){
#if DEBUG
    print_ring_buff();
#endif
    return head == tail;
}

static char ring_get_head(void){
    return ring_buff[head];
}

// |ht| | | | | | | | <
// |h|t| | | | | | |  <
// ...
// | |h|.|.|.|.|.|t| <
// |t|h|.|.|.|.|.|.|
static char ring_pop_head(void){
    char c = ring_buff[head];

    if(head == RING_BUF_LEN-1){
        head = 0;
    }
    else{
        head++;
    }
    return c;
}

static void ring_insert(char c){
    ring_buff[tail] = c;

    if(tail == RING_BUF_LEN-1){
        tail = 0;
    }
    else{
        tail++;
    }

#if DEBUG
    if(tail == head){
        printk(KERN_WARNING "Ring Buffer full, will start overwriting");
    }
#endif
}

// sums up the numbers in ring buffer.
// numbers are assumned to be delimited by
// non-numeric character.
static int sum(void){
    char c;
    int total = 0, num = 0;
    while(!ring_empty()){
        c = ring_pop_head();
        if(c >='0' && c <= '9')
            num = num*10 + (int)(c-'0');
        else{
            // number end
            total += num;
            num = 0;
        }
    }

    return total;
}

static struct file_operations fops = {
 .read = device_read,
 .write = device_write,
 .open = device_open,
 .release = device_release
};

/**
 * @brief This function is called, when the module is loaded into the kernel
 */
static int __init my_init(void) {
    Major = register_chrdev(0, DEVICE_NAME, &fops);
    
    if (Major < 0) {
        printk(KERN_ALERT "Registering char device failed with %d\n", Major);
        return Major;
    }
    init_ring_buf();
    printk(KERN_INFO "Hi there. Major number assigned: %d.\n", Major);
    printk(KERN_INFO "Create a dev file with\n");    
    printk(KERN_INFO "'mknod /dev/%s c %d 0'.\n", DEVICE_NAME, Major);
    
    return 0;
}

/**
 * @brief This function is called, when the module is removed from the kernel
 */
static void __exit my_exit(void) {
    printk("Goodbye, Kernel\n");
    unregister_chrdev(Major, DEVICE_NAME);
}

/**
 * @brief This function is called, when the file is open.
 * If opened in read mode, the sum of data in ring buff is 
 * calculated and written to the buf.
 */
static int device_open(struct inode *inode, struct file *file){
    if(Device_open)
        return -EBUSY;

    Device_open++;

    printk(KERN_INFO "Device open");
    if(file->f_mode & FMODE_READ){
        printk(KERN_INFO "opened for reading");
        int s = sum();

#if DEBUG
        printk(KERN_INFO "sum: %d", s);
#endif
        sprintf(buf, "%d", s);
        msg_Ptr = buf;
    }

    try_module_get(THIS_MODULE);
    return 0;
}

/**
 * @brief This function is called, when the file is closed.
 * Adds a space at the head, to differentiate next write.
 */
static int device_release(struct inode *inode, struct file *file){
    Device_open--;
    module_put(THIS_MODULE);

    ring_insert(' ');
    return 0;
}

/**
 * @brief This function is called, when the user will try to read from a file.
 * Adds a space at the head, to differentiate next write.
 * Writes the answers stored n buff to userspace.
 */
static ssize_t device_read(struct file *file, char __user *buffer, size_t length, loff_t *offset){
    int bytes_read = 0;

    // if at end, return 0
     if (*msg_Ptr == '\0')
         return 0;

     while (length && *msg_Ptr) {
         /*
          * buffer is in userspace
         * put_user is used to copy data to it
         */
         put_user(*(msg_Ptr++), buffer++);
         length--;
        bytes_read++;
     }

    // return number of bytes read
    // 0 indicates no more data is left to read
     return bytes_read;
}

/**
 * @brief This function is called, when the user writes to a file.
 * Inserts the data to ringbuffer.
 */
static ssize_t
device_write(struct file *filp, const char *buff, size_t len, loff_t * off){
    int i = 0;
    char c;

    for (i=0; i < len; i++){
        get_user(c, buff+i);
        ring_insert(c);
    }

    // return the number of bytes copied
    return i;
}

module_init(my_init);
module_exit(my_exit);
