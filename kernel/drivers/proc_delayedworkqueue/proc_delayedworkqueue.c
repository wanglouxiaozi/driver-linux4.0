#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/version.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/seq_file.h>
#include <linux/string.h>
#include <linux/workqueue.h>
#include <linux/delay.h>
#include <linux/device.h>

#define GLOBALMEM_SIZE	0x1000
#define MEM_CLEAR 0x1
#define GLOBALMEM_MAJOR 230

static int globalmem_major = GLOBALMEM_MAJOR;
module_param(globalmem_major, int, S_IRUGO);

struct globalmem_dev {
	struct cdev cdev;
	struct class *cls;
	unsigned char mem[GLOBALMEM_SIZE];
};

struct globalmem_dev *globalmem_devp;

static struct delayed_work dwork;


static unsigned int variable = 1;
//static unsigned char variable[20] = "default\n";
static struct proc_dir_entry *test_dir, *test_entry;

static void do_my_work(struct delay_work *work)
{
	static int count = 0;
	memcpy(&variable, &count, sizeof(int));
	printk("count ++\n");
	count++;
	schedule_delayed_work(&dwork, 1000);
}

static int test_proc_show(struct seq_file *seq, void *v)
{
	/*unsigned char *ptr_var = seq->private;
	seq_printf(seq, "%s\n", ptr_var);
	*/	
	unsigned int *ptr_var = seq->private;
	seq_printf(seq, "%u\n", *ptr_var);

	return 0;
}
/*
   static int run_test(void)
   {
   int a = 1920, b= 12, c= 2132;
   sprintf(variable, "%d\n""%d\n""%d\n", a, b, c);
   return 0;
   }
 */
static ssize_t test_proc_write(struct file *file, const char __user
				*buffer, size_t count, loff_t *ppos)
{
	struct seq_file *seq = file->private_data;
	unsigned int *ptr_var = seq->private;

	*ptr_var = simple_strtoul(buffer, NULL, 10);
	return count;
}

static int test_proc_open(struct inode *inode,
				struct file *file)
{
	return single_open(file, test_proc_show, PDE_DATA(inode));
}

static const struct file_operations test_proc_fops =
{
	.owner = THIS_MODULE,
	.open  = test_proc_open,
	.read  = seq_read,
	.write = test_proc_write,
	.llseek = seq_lseek,
	.release = single_release,
};

static int test_proc_init(void)
{
	test_dir = proc_mkdir("test_dir", NULL);
	if (test_dir) {
		test_entry = proc_create_data("test_rw", 0, test_dir, &test_proc_fops, &variable);
		if (test_entry)
		{
//			run_test();
			return 0;
		}
	}

	return -ENOMEM;
}

static void test_proc_cleanup(void)
{
	remove_proc_entry("test_rw", test_dir);
	remove_proc_entry("test_dir", NULL);
}


static const struct file_operations globalmem_fops = {
	.owner = THIS_MODULE,
	//.llseek = globalmem_llseek,
	//.read = globalmem_read,
	//.write = globalmem_write,
//	.unlocked_ioctl = globalmem_ioctl,
//	.open = globalmem_open,
//	.release = globalmem_release,
};

static void globalmem_setup_cdev(struct globalmem_dev *dev, int index)
{
	int err, devno = MKDEV(globalmem_major, index);

	cdev_init(&dev->cdev, &globalmem_fops);
	dev->cdev.owner = THIS_MODULE;
	err = cdev_add(&dev->cdev, devno, 1);
	if (err)
		printk(KERN_NOTICE "Error %d adding globalmem%d", err, index);
}

static int __init globalmem_init(void)
{
	int ret;
	dev_t devno = MKDEV(globalmem_major, 0);

	if (globalmem_major)
		ret = register_chrdev_region(devno, 1, "globalmem");
	else {
		ret = alloc_chrdev_region(&devno, 0, 1, "globalmem");
		globalmem_major = MAJOR(devno);
	}
	if (ret < 0)
		return ret;

	globalmem_devp = kzalloc(sizeof(struct globalmem_dev), GFP_KERNEL);
	if (!globalmem_devp) {
		ret = -ENOMEM;
		goto fail_malloc;
	}

	globalmem_setup_cdev(globalmem_devp, 0);
	globalmem_devp->cls = class_create(THIS_MODULE, "my_class");
	if (IS_ERR(globalmem_devp->cls)) {
		printk("Err: failed in creating class\n");
		return -1;
	}
	device_create(globalmem_devp->cls, NULL, MKDEV(globalmem_major, 0), NULL, "globalmem%d", 0);
	test_proc_init();
	INIT_DELAYED_WORK(&dwork, do_my_work);
	schedule_delayed_work(&dwork, 1000);

	return 0;

 fail_malloc:
	unregister_chrdev_region(devno, 1);
	return ret;
}
module_init(globalmem_init);

static void __exit globalmem_exit(void)
{
	test_proc_cleanup();
	cancel_delayed_work(&dwork);
	cancel_delayed_work_sync(&dwork);
	cdev_del(&globalmem_devp->cdev);
	device_destroy(globalmem_devp->cls, MKDEV(globalmem_major, 0));
	class_destroy(globalmem_devp->cls);
	kfree(globalmem_devp);
	unregister_chrdev_region(MKDEV(globalmem_major, 0), 1);
}
module_exit(globalmem_exit);

MODULE_AUTHOR("allenwang@tvunetworks.com");
MODULE_LICENSE("GPL v2");
