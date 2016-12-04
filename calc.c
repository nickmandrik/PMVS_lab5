#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("Calculator");
MODULE_AUTHOR("Nick Mandrik");
MODULE_VERSION("1");

#define PROC_FIRST "first"
#define PROC_SECOND "second"  
#define PROC_OPERAND "operand"
#define DEV_RESULT "result"

#define PROC_MAX_SIZE 20

static struct proc_dir_entry *first_proc_file, *second_proc_file, *operand_proc_file;
static char first_buffer[PROC_MAX_SIZE], second_buffer[PROC_MAX_SIZE], operand_buffer[PROC_MAX_SIZE];
static unsigned long first_buffer_size = 0, second_buffer_size = 0, operand_buffer_size = 0;
static const int first_proc_index = 1, second_proc_index = 2, operand_proc_index = 3;

static ssize_t proc_write(struct file *filp, const char __user *buffer, size_t count, loff_t *data)
{
	char *proc_buffer;
	unsigned long* buffer_size;
	const unsigned char *name = filp->f_path.dentry->d_name.name;

	printk(KERN_INFO "proc_write called\n");


	if (strcmp(name, PROC_FIRST) == 0) {
		printk(KERN_INFO "proc_write for first\n");
		proc_buffer = first_buffer;
		buffer_size = &first_buffer_size;
	} else if (strcmp(name, PROC_SECOND) == 0) {
		printk(KERN_INFO "proc_write for second\n");
		proc_buffer = second_buffer;
		buffer_size = &second_buffer_size;
	} else if (strcmp(name, PROC_OPERAND) == 0) {
		printk(KERN_INFO "proc_write for operand\n");
		proc_buffer = operand_buffer;
		buffer_size = &operand_buffer_size;
	} else {
		return 0;
	}

	*buffer_size = count;
	if (*buffer_size > PROC_MAX_SIZE ) {
		*buffer_size = PROC_MAX_SIZE;
	}
	if ( copy_from_user(proc_buffer, buffer, *buffer_size) ) {
		return -EFAULT;
	}
	
	return *buffer_size;
}

int str_to_int(char *str, int n) 
{
	int len = 0;
	char *s = str;
	int k = 1;
	int res = 0;
	int i = 0;
	int negative = 0;
	if (*s == '-') {
		negative = 1;
		s++;
		n--;
	}
	while (*s >= '0' && *s <= '9' && len < n) {
		len ++, s ++;
	}
	--s;
	for (i = 0; i < len; i++, s--, k *= 10) {
		res += k * (*s - '0');
	}
	if (negative) {
		res *= -1;
	}
	return res;
}


int int_len(int num)
{
	int len = 0;
	int k = 1;
	if (num == 0) {
		return 1;
	}
	if (num < 0) {
		len++;
		num *= -1;
	}
	while(num / k > 0) {
		len++;
		k *= 10;
	}
	return len;
}

static ssize_t dev_read(struct file * file, char * buf, size_t count, loff_t *ppos)
{
	char result[PROC_MAX_SIZE];
	int len = 0;
	int first = str_to_int(first_buffer, first_buffer_size);
	int second = str_to_int(second_buffer, second_buffer_size);
	int ok = 1;
	int res = 0;
	if (operand_buffer_size == 0) {
		ok = 0;
	} else {
		switch(operand_buffer[0]) {
			case '-':
				res = first - second;
				break;
			case '+':
				res = first + second;
				break;
			case '*':
				res = first * second;
				break;
			case '/':
				if (second == 0) {
					ok = 0;
					break;
				}
				res = first / second;
				break;
			default:
				ok = 0;
		}
	}
	if (!ok) {
		memcpy(result, "Error!", 6);
		len = 6;
	} else {
		sprintf(result, "%d", res);
		len = int_len(res);
	}
	if (count < len)
		return -EINVAL;
	if (*ppos != 0)
		return 0;
	if (count < len)
		return -EINVAL;
	if (*ppos != 0)
		return 0;
	if (copy_to_user(buf, result, len))
		return -EINVAL;
	*ppos = len;
	return len;
}

static struct file_operations proc_file_ops = {
	.owner = THIS_MODULE,
	.write = proc_write,
};
static const struct file_operations dev_file_ops = {
	.owner = THIS_MODULE,
	.read = dev_read,
};
static struct miscdevice result_dev = {
	MISC_DYNAMIC_MINOR,
	DEV_RESULT,
	&dev_file_ops
};


static int calc_init(void)
{		
	printk(KERN_INFO "Calc module started working\n");
	first_proc_file = proc_create_data(PROC_FIRST, 766, NULL, &proc_file_ops, (void*) &first_proc_index);
	if (first_proc_file == NULL) {
		printk(KERN_ERR "can't create first proc");
		remove_proc_entry(PROC_FIRST, NULL);
		return -ENOMEM;
	}
	second_proc_file = proc_create_data(PROC_SECOND, 766, NULL, &proc_file_ops, (void*) &second_proc_index);
	if (second_proc_file == NULL) {
		printk(KERN_ERR "can't create second proc");
		remove_proc_entry(PROC_FIRST, NULL);
		remove_proc_entry(PROC_SECOND, NULL);
		return -ENOMEM;
	}
	operand_proc_file = proc_create_data(PROC_OPERAND, 766, NULL, &proc_file_ops, (void*) &operand_proc_index);
	if (operand_proc_file == NULL) {
		printk(KERN_ERR "can't create operand proc");
		remove_proc_entry(PROC_FIRST, NULL);
		remove_proc_entry(PROC_SECOND, NULL);
		remove_proc_entry(PROC_OPERAND, NULL);
		return -ENOMEM;
	}	
	if (misc_register(&result_dev)) {
		printk(KERN_ERR "Unable to register \"result\" misc device\n");
		remove_proc_entry(PROC_FIRST, NULL);
		remove_proc_entry(PROC_SECOND, NULL);
		remove_proc_entry(PROC_OPERAND, NULL);
		return -ENOMEM;
	}
	return 0;
}

static void calc_exit(void)
{
	printk(KERN_INFO "Calc module stopped working\n");
	remove_proc_entry(PROC_FIRST, NULL);
	remove_proc_entry(PROC_SECOND, NULL);
	remove_proc_entry(PROC_OPERAND, NULL);
	misc_deregister(&result_dev);
}


module_init(calc_init);
module_exit(calc_exit);
