#include <stdio.h>
#include <stdlib.h>

void launch_SuperNET()
{
    if ( system("./SuperNET &") != 0 )
        printf("error launching SuperNET\n");
}
