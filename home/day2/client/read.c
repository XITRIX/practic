#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

int main(){
	int fl;
	unsigned int buff;

	if ((fl = open("/dev/blacknode",O_RDONLY)) == -1){
    	printf("OPENING ERROR\n");
      	exit(1);
   	}

	if((read(fl,&buff,sizeof(unsigned int)))==-1)
   	{
    	printf("READING ERROR\n");
      	exit(1);
   	}

   	printf("READED: %d\n",buff);

	return 0;
}