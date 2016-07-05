#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]){
	int fl;
    unsigned int buff;
	if (argc > 1) buff = atoi(argv[1]);
    else buff = 12;

	if ((fl = open("/dev/blacknode",O_WRONLY)) == -1){
    	printf("OPENING ERROR\n");
      	exit(1);
   	}

	if((write(fl,&buff,sizeof(unsigned int)))==-1)
   	{
    	printf("WRITING ERROR\n");
      	exit(1);
   	}

   	printf("WRITED: %d\n",buff);

	return 0;
}