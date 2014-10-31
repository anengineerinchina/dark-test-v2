#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
void *poll_for_broadcasts(void *args);
extern int32_t SuperNET_retval,did_SuperNET_init;

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

void *_launch_SuperNET(void *_myip)
{
    char *myip = _myip;
    FILE *fp;
    char cmd[128];
    int32_t retval;
    void *processptr = 0;
    system("rm horrible.hack");
    sprintf(cmd,"./SuperNET %s &",myip);
    if ( system(cmd) != 0 )
        printf("error launching (%s)\n",cmd);
    while ( (fp= fopen("horrible.hack","rb")) == 0 )
        sleep(1);
    if ( fread(&retval,1,sizeof(retval),fp) != sizeof(retval) )
        retval = -2;
    fclose(fp);
    printf("SuperNET file found! retval.%d\n",retval);
    if ( retval == 0 )
    {
        processptr = portable_thread_create(poll_for_broadcasts,0);
        did_SuperNET_init = 1;
    }
    SuperNET_retval = retval;
    return(processptr);
}

int32_t launch_SuperNET(char *myip)
{
    void *processptr;
    processptr = portable_thread_create(_launch_SuperNET,myip);
    return(0);
}