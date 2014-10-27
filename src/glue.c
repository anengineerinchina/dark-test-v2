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

void launch_SuperNET()
{
    FILE *fp;
    void *processptr;
    system("rm horrible.hack");
    processptr = portable_thread_create(poll_for_broadcasts,0);
    if ( system("./SuperNET &") != 0 )
        printf("error launching SuperNET\n");
    while ( (fp= fopen("horrible.hack","rb")) == 0 )
        sleep(1);
    fclose(fp);
    printf("SuperNET file found!\n");
}