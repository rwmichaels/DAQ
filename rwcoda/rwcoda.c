/* Program to read a CODA file
*/
    

#include <stdio.h>
#include <cstdlib>
#include <cstring>

#define MAXEVLEN 819200
#define S_SUCCESS 0
#define S_EVFILE_TRUNC  0x40730001      /* Event truncated on read */

#include "evio.h"

main (int argc, char *argv[])
{
  int handle1,handle2;
    uint32_t evbuffer[MAXEVLEN];
    uint32_t ev_size,evlen,status,ev_type,evnum,nseen,nevent,go_on;
    int lprint = 1;
    int jj;
    char inputfile[80];

    printf("argc = %d \n",argc);
    if( argc!=2 && argc!=3 ) {
       printf("You made a mistake... \n");
       printf("Usage: rwcoda file-in [nevent] \n\n");
       printf("where file-in is the input CODA file\n");
       exit(1);
    }
    strcpy(inputfile,argv[1]);
    printf("\n CODA file: %s  \n",inputfile);
    nevent = 1e8;
    if(argc>2) {  
       nevent = atoi(argv[2]);
    }
    if(evOpen(inputfile,"r",&handle1)!=0) {
       printf("File '%s' not found.  Quitting.\n",inputfile);
       exit(1);
    }

/* Filter loop */

    go_on=1;
    evnum=0; 
    nseen=0;
    while (go_on && nseen < nevent) {
         ev_size = sizeof(evbuffer)>>2;
         status=evRead(handle1,evbuffer,ev_size);
         if(status!=S_SUCCESS) {
           if(status==S_EVFILE_TRUNC) {
             printf("ERROR: Event %d truncated\n",nevent);
             printf("Probably evbuffer too small\n");
           } else {
             if(status!=EOF) {
               printf("\n Unexpected evRead status 0x%x \n",status);  
	     }
             go_on=0;
	   }
	 }
         nseen++;
         printf("stuff %d %d %d %d \n",nseen,go_on,evnum,nevent);
         if(!go_on) break;
         evlen = evbuffer[0];
         ev_type  = evbuffer[1]>>16;
         evnum = evbuffer[4];

         if (lprint) { /* print the event */
             printf("\n\n -------  Event type %d ---------------- \n",ev_type);
             for (jj=0; jj < evlen+1; jj++) {

	       printf("evbuffer[%d] = %d  = 0x%x \n",jj,evbuffer[jj],evbuffer[jj]);

	     }
	 }

    }
    evClose(handle1);
}




