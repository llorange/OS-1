#define __LIBRARY__

#include <errno.h>
#include <unistd.h>
#include <string.h>

#include <linux/sched.h>
#include <linux/kernel.h>
#include <asm/segment.h>

#include <sys/types.h>
#include <sys/stat.h>

//设定seconds秒闹钟
int sys_sleep(unsigned int seconds){
	sys_signal(SIGALRM , SIG_IGN);//收到SIGALRM信号，则SIG_IGN忽略
	sys_alarm(seconds);
	sys_pause();// 来暂停当前进程,进程进入睡眠态
	// 信号函数成功返回则pause函数返回，返回值-1
	return 0; 
}

