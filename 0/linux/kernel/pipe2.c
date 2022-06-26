#define __LIBRARY__
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>

#include <linux/kernel.h>
#include <linux/sched.h>
#include <asm/segment.h>
#include <sys/stat.h>

int sys_pipe2(int * pipefd,int flags)
{
    sys_pipe(pipefd);
    return 0;
}