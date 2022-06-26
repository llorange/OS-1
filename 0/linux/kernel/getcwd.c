#define __LIBRARY__
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>

#include <linux/kernel.h>
#include <linux/sched.h>
#include <asm/segment.h>
#include <sys/stat.h>

#define BUF_MAX 256

char * sys_getcwd(char * buf, size_t size)
{	
	char temp[BUF_MAX], buff[BUF_MAX];
	int flag,len;
	struct m_inode *current_inode = current->pwd;
	struct m_inode *root_inode = current->root;
	struct super_block *super_b = (struct super_block *)bread(current_inode->i_dev,1);// 超级块
	struct m_inode *base_inode  = super_b->s_isup;	// 索引节点基址
	struct m_inode *f_inode;
	struct dir_entry *f_entrys;
	struct dir_entry *current_entrys;
	int i;

	while(1)
	{
		//指向当前目录下的某一目录项
		current_entrys =  (struct dir_entry *)bread(current_inode->i_dev,current_inode->i_zone[0])->b_data;
		int current_entrys_num = current_inode->i_size/sizeof(struct dir_entry);//当前目录的目录项数量
		//在当前目录中找到父目录的目录项
		flag = 0;
		for(i=0;i<current_entrys_num;i++)
		{
			if(!strcmp((current_entrys+i)->name,".."))
			{//通过目录项的名字搜索父目录
				flag = 1;
				f_inode = iget(current_inode->i_dev,(current_entrys + i)->inode);//父目录i节点
				break;
			}
		}
		if(!flag) 
		{
			printk("Sorry,No ..\n");
			return NULL;
		}
		
		f_entrys = (struct dir_entry *)bread(f_inode->i_dev,f_inode->i_zone[0])->b_data;
		int father_entrys_num = f_inode->i_size/sizeof(struct dir_entry);//父目录的目录项数量
		//在父目录中寻找当前目录的目录项
		flag = 0;
		for(i=0;i<father_entrys_num;i++)
		{
			if((f_entrys+i)->inode == current_inode->i_num)
			{//通过目录项的i节点序号搜索子目录
				memset(temp, 0, sizeof(temp));
				strcat(temp,"/");
				strcat(temp,(f_entrys+i)->name);
				strcat(temp,buff);
				strncpy(buff,temp,BUF_MAX);
				flag = 1;
				break;
			}
		}
		if(!flag){
			printk("Sorry, Current dircory is lost\n");
			return NULL;
		}	
		if(root_inode->i_num == f_inode->i_num)
			break;
		current_inode = f_inode;
	}
	//将位于核心段的buff拷贝到位于用户段的buf
	len = 0;
	while(buff[len++])//计算cwd字符串长度
	;
	for(i=0;i<len;i++)
	{
		put_fs_byte(buff[i],buf+i);
	}
	put_fs_byte('\0',buf+len);
	return buf;
}