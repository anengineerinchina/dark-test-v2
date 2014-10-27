#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
void *poll_for_broadcasts(void *args);

void *portable_thread_create(void *funcp,void *argp)
{
    pthread_t *ptr;
    ptr = (pthread_t *)malloc(sizeof(*ptr));
    if ( pthread_create(ptr,NULL,funcp,argp) != 0 )
    {
        free(ptr);
        return(0);
    } else return(ptr);
}

void launch_SuperNET(char *myip)
{
    FILE *fp;
    char cmd[128];
    void *processptr;
    system("rm horrible.hack");
    sprintf(cmd,"./SuperNET %s &",myip);
    if ( system(cmd) != 0 )
        printf("error launching (%s)\n",cmd);
    while ( (fp= fopen("horrible.hack","rb")) == 0 )
        sleep(1);
    fclose(fp);
    printf("SuperNET file found!\n");
    processptr = portable_thread_create(poll_for_broadcasts,0);
}