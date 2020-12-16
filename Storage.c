#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/string.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/kdev_t.h>
#include <linux/uaccess.h>
#include <linux/errno.h>
#define BUFF_SIZE 20

MODULE_LICENSE("Dual BSD/GPL");

dev_t my_dev_id;
static struct class *my_class;
static struct device *my_device;
static struct cdev *my_cdev;

int storage[10];
int pos = 0;
int endRead = 0;

int rega, regb, regc, regd, result;

int storage_open(struct inode *pinode, struct file *pfile);
int storage_close(struct inode *pinode, struct file *pfile);
ssize_t storage_read(struct file *pfile, char __user *buffer, size_t length, loff_t *offset);
ssize_t storage_write(struct file *pfile, const char __user *buffer, size_t length, loff_t *offset);

struct file_operations my_fops =
{
	.owner = THIS_MODULE,
	.open = storage_open,
	.read = storage_read,
	.write = storage_write,
	.release = storage_close,
};


int storage_open(struct inode *pinode, struct file *pfile) 
{
		printk(KERN_INFO "Succesfully opened file\n");
		return 0;
}

int storage_close(struct inode *pinode, struct file *pfile) 
{
		printk(KERN_INFO "Succesfully closed file\n");
		return 0;
}

ssize_t storage_read(struct file *pfile, char __user *buffer, size_t length, loff_t *offset) 
{
	int ret;
	char buff[BUFF_SIZE];
	long int len;
	if (endRead){
		endRead = 0;
		pos = 0;
		printk(KERN_INFO "Succesfully read from file\n");
		return 0;
	}
	len = scnprintf(buff,BUFF_SIZE , "%d ", storage[pos]);
	ret = copy_to_user(buffer, buff, len);
	if(ret)
		return -EFAULT;
	pos ++;
	if (pos == 10) {
		endRead = 1;
	}
	return len;
}

ssize_t storage_write(struct file *pfile, const char __user *buffer, size_t length, loff_t *offset) 
{
	char buff[BUFF_SIZE];
	int value;
	int ret;
	char reg;

	ret = copy_from_user(buff, buffer, length);
	if(ret)
		return -EFAULT;
	buff[length-1] = '\0';

	ret = sscanf(buff,"reg%c=%x",&reg,&value);

	if(ret==2)//two parameters parsed in sscanf
	{
		if(value < 0 || value >255){
			printk(KERN_WARNING "Broj treba da bude izmedju 0x00 i 0xff\n");
			return -1;
		}else{
			switch(reg){
				case 'a':
					rega=value;
					break;
				case 'b':
					regb=value;
					break;
				case 'c':
					regc=value;
					break;
				case 'd':
					regd=value;
					break;
				default:
					printk(KERN_WARNING "Registar moze da bude a, b, c ili d\n");
					return -1;

			};
			printk(KERN_INFO "reg%c -> 0x%x",reg ,value);
		}
	}
	else
	{	char reg1, reg2, op, val1, val2;
		ret = sscanf(buff, "reg%c %c reg%c", &reg1, &op, &reg2);
		if(ret == 3){
			printk(KERN_INFO "reg%c i reg%c sa operacijom %c\n", reg1, reg2, op);
			switch(reg1){
				case 'a':
					val1 = rega;
					break;
				case 'b':
					val1 = regb;
					break;
				case 'c':
					val1 = regc;
					break;
				case 'd':
					val1 = regd;
					break;
				default:
					printk(KERN_WARNING "Reg moze da bude a,b,c ili d\n");
					return -1;
			};

			switch(reg2){
				case 'a':
					val2 = rega;
					break;
				case 'b':
					val2 = regb;
					break;
				case 'c':
					val2 = regc;
					break;
				case 'd':
					val2 = regd;
					break;
				default:
					printk(KERN_WARNING "Reg moze da bude a,b,c ili d\n");
					return -1;
			};

			switch(op){
				case '+':
					result = val1+val2;
					break;
				case '-':
					result = val1-val2;
					break;
				case '*':
					result = val1*val2;
					break;
				case '/':
					if(val2 != 0){
					result = val1/val2;
					}else{
						printk(KERN_WARNING "Deljenje sa nulom nije dozvoljeno\n");
					}
					break;
				default:
					printk(KERN_WARNING "Dozvoljene operacije su +, -, * i /\n");
					return -1;
			};

			printk(KERN_INFO "rezultat je %d", result);
			return result;
			
		}else{
		printk(KERN_WARNING "Pogresan format komande\nTreba da bude regx=broj ili regx ? regy\n");
		}
	}

	return length;
}

static int __init storage_init(void)
{
   int ret = 0;
	int i=0;

	//Initialize array
	for (i=0; i<10; i++)
		storage[i] = 0;

   ret = alloc_chrdev_region(&my_dev_id, 0, 1, "storage");
   if (ret){
      printk(KERN_ERR "failed to register char device\n");
      return ret;
   }
   printk(KERN_INFO "char device region allocated\n");

   my_class = class_create(THIS_MODULE, "storage_class");
   if (my_class == NULL){
      printk(KERN_ERR "failed to create class\n");
      goto fail_0;
   }
   printk(KERN_INFO "class created\n");
   
   my_device = device_create(my_class, NULL, my_dev_id, NULL, "storage");
   if (my_device == NULL){
      printk(KERN_ERR "failed to create device\n");
      goto fail_1;
   }
   printk(KERN_INFO "device created\n");

	my_cdev = cdev_alloc();	
	my_cdev->ops = &my_fops;
	my_cdev->owner = THIS_MODULE;
	ret = cdev_add(my_cdev, my_dev_id, 1);
	if (ret)
	{
      printk(KERN_ERR "failed to add cdev\n");
		goto fail_2;
	}
   printk(KERN_INFO "cdev added\n");
   printk(KERN_INFO "Hello world\n");

   return 0;

   fail_2:
      device_destroy(my_class, my_dev_id);
   fail_1:
      class_destroy(my_class);
   fail_0:
      unregister_chrdev_region(my_dev_id, 1);
   return -1;
}

static void __exit storage_exit(void)
{
   cdev_del(my_cdev);
   device_destroy(my_class, my_dev_id);
   class_destroy(my_class);
   unregister_chrdev_region(my_dev_id,1);
   printk(KERN_INFO "Goodbye, cruel world\n");
}


module_init(storage_init);
module_exit(storage_exit);
