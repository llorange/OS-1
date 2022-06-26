#define __LIBRARY__
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>

#include <linux/kernel.h>
#include <linux/sched.h>
#include <asm/segment.h>
#include <sys/stat.h>

#define NAME_MAX     14    //目录项16个字节，前2个字节是索引节点号

// 获取⽬录的⽬录项

// 定义目录项结构体用于输出
struct linux_dirent {
    long           d_ino;
    off_t          d_off;
    unsigned short d_reclen;
    char           d_name[NAME_MAX+1];
};

int sys_getdents(unsigned int fd, struct linux_dirent *dirp, unsigned int count)
{
    struct file         *file;   
    struct m_inode      *inode; 
    struct dir_entry 	*dir_ey;     	// 目录项(内存)
	struct buffer_head  *buf_hd;        // 缓冲块头

    int linux_dirent_size = sizeof(struct linux_dirent);
    int dir_entry_size = sizeof(struct dir_entry);
    int cnt_l = 0;// 已读linux_dirent总字节数
    int cnt_d = 0;// 已读页表项的总字节数

	// 目录的文件描述符错误
    if(fd >= NR_OPEN)
		return -1;    
	      
	if(count < linux_dirent_size)
		return -1;

    file = current->filp[fd];
	if(!file)
		return -1;                    

    inode = file->f_inode;              // 目录的索引节点
    struct linux_dirent *dirp_tmp;      // 专为输出的目录项结构,为dirp的赋值而开
    dirp_tmp = (struct linux_dirent *)malloc(linux_dirent_size);
    char * buf;                         // 为dirp的赋值而开，大小为一个linux_dirent
    buf = (char*)malloc(linux_dirent_size);

    buf_hd = bread(inode->i_dev , inode->i_zone[0]);// 读目录索引节点中第一个数据块

    for( ; cnt_d < inode->i_size ; cnt_d += dir_entry_size)
    {
        if (cnt_l >= count - linux_dirent_size)// dirp已满，则返回读取的字节数
			break;     
        
        // bufhd->b_data 是char*类型的指针（1字节），+d得到该目录项
        dir_ey = (struct dir_entry *)(buf_hd->b_data + cnt_d);
        // 读到目录的最后一项返回0
        if(!dir_ey->inode)
			continue;
        
        // 将目录项信息从dir_entry转移到dirp_tmp
        dirp_tmp->d_ino = dir_ey->inode;
        dirp_tmp->d_off = 0; 
        dirp_tmp->d_reclen = linux_dirent_size;
        strcpy(dirp_tmp->d_name,dir_ey->name);

        // 将dirp_tmp的信息转化成char类型，存入buf
        memcpy(buf,dirp_tmp,linux_dirent_size);

        // 将核心数据段中的buf逐字节存入用户数据段中的dirp
		// 段基址fs  
        for(int i=0 ; i < linux_dirent_size ; i++){
            put_fs_byte(*(buf+i), ((char*)dirp)+cnt_l+i);
        }
        cnt_l += linux_dirent_size;
    }
	return cnt_l;
}