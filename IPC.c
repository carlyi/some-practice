#include<unistd.h>
#include<string.h>
#include<stdio.h>
#include<fcntl.h>

#if 0
int main()
{
    char input[] = "IPC by pipe";
    char output[BUFSIZ+1];
    int fd_pipe[2];
    int count = 0;
    int stat_value;
    pid_t pid,pid_res;

    memset(output,'\0',sizeof(output));

    if(pipe(fd_pipe) == 0) // create pipe
    {
	pid = fork();
	if(pid > 0)
	{
	    pid_res = wait(&stat_value);

	    if(pid_res > 0)
	    {
		count = read(fd_pipe[0],output,BUFSIZ);       // read the data from pipe
		printf("father process read %d characters,they are: %s \n",count,output);
	    }
	}
	else if(pid == 0)
	{
	    count = write(fd_pipe[1],input,strlen(input));    // write the dato into pipe
	    printf("son process write %d characters,they are : %s \n",count,input);
	}
	else
	{
	    printf("create process failed \n");
	    return 1;
	}
    }

    return 0;
}
#endif

#if 0
int main()
{
    char buffer[BUFSIZ+1];
    int read_count = 0;
    FILE *p_read,*p_write;

    memset(buffer,'\0',sizeof(buffer));

    p_read = popen("env","r");
    if(NULL == p_read)
    {
	printf("open pipe for reading failed \n");
	return 1;
    }

    p_write = popen("grep bash","w");
    if(NULL == p_write)
    {
	printf("open pipe for writing failed \n");
	return 1;
    }

    read_count = fread(buffer,sizeof(char),BUFSIZ,p_read);
    while(read_count > 0)
    {
	fwrite(buffer,sizeof(char),strlen(buffer),p_write);
	read_count = fread(buffer,sizeof(char),BUFSIZ,p_read);
    }

    pclose(p_read);
    pclose(p_write);

    return 0;
}
#endif

int main()
{
    char input[] = "IPC by pipe";
    char output[BUFSIZ+1];
    char p_name[] = "/tmp/test_fifo";
    int count = 0;
    int fd;
    int stat_value;
    pid_t pid,pid_res;

    memset(output,'\0',sizeof(output));

    if(mkfifo(p_name,0777) == 0) // create pipe
    {
	pid = fork();
	if(pid > 0)
	{
	    printf("father running \n");
	    fd = open(p_name,O_RDONLY); //open by read mode
	    if(fd == -1)
	    {
		printf("open pipe file failed \n");
		return 1;
	    }
	}
	else if(pid == 0)
	{
	    printf("son running \n");
	    fd = open(p_name,O_WRONLY); //open by write mode
	    if(fd == -1)
	    {
		printf("open pipe file failed \n");
		return 1;
	    }

	    count = write(fd,input,strlen(input));    // write the dato into pipe
	    printf("son process write %d characters,they are : %s \n",count,input);
	    close(fd);
	}
	else
	{
	    printf("create process failed \n");
	    return 1;
	}
    }
    else
    {
	printf("create fifo failed \n");
	return 1;
    }

    pid_res = wait(&stat_value);

    if(pid_res > 0)
    {
	count = read(fd,output,BUFSIZ);       // read the data from pipe
	printf("father process read %d characters,they are: %s \n",count,output);

	close(fd);
    }

    return 0;
}
