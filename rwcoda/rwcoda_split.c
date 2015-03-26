/* Program to read a CODA file and split it into sizes of NEVENT events,
   aimed to make files of about 2 GByte */

/* R. Michaels, March, 2015 */

#include <stdio.h>
#include <cstdlib>
#include <cstring>

#define FILTERTYPE 0
#define MAXEVLEN 819200
/* 40K events is about 2.2 GByte for DVCS setup.  */
#define NEVENT_SPLIT 40000
#define S_SUCCESS 0
#define S_EVFILE_TRUNC  0x40730001      /* Event truncated on read */
#define MAXOUT 100

#include "evio.h"

main (int argc, char *argv[])
{
    int handle1,handle2;
    uint32_t evbuffer[MAXEVLEN];
    uint32_t ev_size,evlen,status,ev_type,evnum,nevent,nevtot;
    int lprint = 0;
    int jj,iout,go_on;
    char inputfile[80],outputfile[80],csuffix[40];
    char evlist[]="eventlist.dat";
    FILE *fp;


    if( argc!=2 ) {
       printf("You made a mistake... \n");
       printf("Usage: rwcoda file-in \n\n");
       printf("where file-in is the input CODA file\n");
       exit(1);
    }
    strcpy(inputfile,argv[1]);
    printf("\n CODA file: %s  \n",inputfile);
    if(evOpen(inputfile,"r",&handle1)!=0) {
       printf("File '%s' not found.  Quitting.\n",inputfile);
       exit(1);
    }

    ev_size = sizeof(evbuffer)>>2;
    nevtot=0;

    for (iout=0; iout<MAXOUT; iout++) {

      strcpy(outputfile, inputfile);
      sprintf(csuffix,"_split%d",iout);
      strcat(outputfile,csuffix);
      printf("output file %s \n",outputfile);
      if(strcmp(inputfile,outputfile)==0) {
	/* shouldn't happen -- a bit of extra safety */
         printf("Error -- input file cannot be same as output \n");
         exit(1);
      }
      if((fp = fopen(outputfile,"r"))!=NULL) {
	/* shouldn't happen -- a bit of extra safety */
         printf("Error -- output file already exists !\n");
         exit(1);
      }
      if(evOpen(outputfile,"w",&handle2)!=0) {
         printf("Error opening file %s.  Quitting.\n",outputfile);
         exit(1);
      }

      go_on=1;
      nevent=0;
      printf("Writing file #%d  filename = %s \n",iout+1,outputfile);

      while (go_on) {

         status=evRead(handle1,evbuffer,ev_size);
         if(status!=S_SUCCESS) {
           if(status==S_EVFILE_TRUNC) {
             printf("ERROR: Event %d truncated\n",nevent);
             printf("Probably evbuffer too small\n");
           } else {
             if(status!=EOF) printf("\n Unexpected evRead status 0x%x \n",status);  
             go_on=0;
             printf("End of File reached \n");
	   }
	 }
         if(!go_on) goto finish1;
         nevent++;
         nevtot++;
         evlen = evbuffer[0];
         ev_type  = evbuffer[1]>>16;
         evnum = evbuffer[4];
         if (lprint) { /* print the event */
	   printf("event %d  type %d  evnum %d ------------------------\n",nevent,ev_type,evnum);
             for (jj=0; jj < evbuffer[0]+1; jj++) {
	       printf("evbuffer[%d] = %d  = 0x%x \n",jj,evbuffer[jj],evbuffer[jj]);
	     }
	 }

          evWrite(handle2,evbuffer);
          if (nevent >=  NEVENT_SPLIT) {
             evClose(handle2);
             go_on=0;
	  }
      } /* go_on while loop */

    } /* loop over output files */

finish1:

    evClose(handle1);

    printf("Finished splitting files \n");
    printf("num files : %d \n",iout);
    printf("num events: %d \n",nevtot);

}




