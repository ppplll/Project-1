/* Created by Andrew K */



#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <poll.h>
#include <signal.h>
#include <errno.h>
#include <assert.h>





int * processbuffer(char * buffer)
{
	
	int i = 0;
	for(;i<strlen(buffer);i++)
	{
		if(('#' == buffer[i]) || ('\n' == buffer[i]))
		{
			buffer[i] = '\0';
		}
	}

	
	char *tempprocessedbuffer[1025];
	i = 1;
	char * temp;	
	temp=strtok(buffer,";");	
	tempprocessedbuffer[0] = temp;
	for(;NULL != temp; i++)
	{
		temp = strtok(NULL,";");
		tempprocessedbuffer[i] = temp; 
	}




	char ** processedbuffer= malloc((1025)*sizeof(char *));
	int procitr = 0;
	int tempitr = 0;
	for(;NULL != tempprocessedbuffer[tempitr];tempitr++)
	{
		i=0;
		char *curtok = tempprocessedbuffer[tempitr];		
		for(;'\0' != curtok[i];i++)
		{
			if ((' ' != curtok[i]) && ('\t' != curtok[i]))   
			{
				processedbuffer[procitr] = curtok;
				procitr++;
				break;
			}
		}
	}
	processedbuffer[procitr]=NULL;	
	void * bufptr= &processedbuffer;
	return bufptr;


}

int seqmode(char * splitprocbuff[1025][1025], int * curcmd)
{
	int flag = 'S';	
	while('S'==flag) /* One time through the while look processes on command*/
	{
		if (NULL == splitprocbuff[*curcmd][0]) /* end of command line */
		{
			flag = 'Q';		
			break;
		}		
		else if (0==strcmp("exit",splitprocbuff[*curcmd][0])) /* exit command */
		{
			flag= 'E';
			break;
		}	
		else if (0==strcmp("mode",splitprocbuff[*curcmd][0])) /* the mode command is called */
		{	
			if (NULL == splitprocbuff[*curcmd][1])
			{			
				printf("The current mode is Sequential\n");
				(*curcmd)++;
			}
			else if ((0==strcmp("sequential",splitprocbuff[*curcmd][1])) || (0==strcmp("s",splitprocbuff[*curcmd][1])))
				(*curcmd)++;
			else if ((0==strcmp("parallel",splitprocbuff[*curcmd][1])) || (0==strcmp("p",splitprocbuff[*curcmd][1])))
			{
				flag = 'P';
				(*curcmd)++;
				printf("Mode Changed To Parallel Mode\n");
			}
			else
			{			
				printf("ERROR: Unrecognized Mode Command\n");
				(*curcmd)++;
			}
		}
		else /* We have something that we are going to interpret as a program to executed */
		{
			pid_t id = fork();
			if (id==0)
			{
				if (execv(splitprocbuff[*curcmd][0],splitprocbuff[*curcmd])<0)
				{
					fprintf(stderr,"ERROR: Failed to Execute File: %s\n", strerror(errno));
					exit(0);		
				}
			}
			else if (id >0)
			{
				int rstatus = 0;
				pid_t childp = wait(&rstatus);
				assert (id==childp);
			}
			else 
				fprintf(stderr,"Failure to Fork: %s\n", strerror(errno));
			
			(*curcmd)++;
		}

	}
	return flag;
}


void parmodewait(int waitnum)
{
	int i =0;
	int rstatus;
	pid_t childp;	
	
	for (;i != waitnum;i++)
	{
		rstatus=0;
		childp = wait(&rstatus);
	}
}

int parmode(char * splitprocbuff[1025][1025], int * curcmd)
{
	int flag = 'P';	
	int waitnum = 0;
	int i =0;	
	while('P'==flag) /* One time through the while look processes on command*/
	{
		if (NULL == splitprocbuff[*curcmd][0]) /* end of command line */
		{
			parmodewait(waitnum);
			flag = 'L';		
			break;
		}		
		else if (0==strcmp("exit",splitprocbuff[*curcmd][0])) /* exit command */
		{
			parmodewait(waitnum);
			flag= 'E';
			break;
		}	
		else if (0==strcmp("mode",splitprocbuff[*curcmd][0])) /* the mode command is called */
		{	
			if (NULL == splitprocbuff[*curcmd][1])
			{			
				printf("The current mode is Parallel\n");
				(*curcmd)++;
			}
			else if ((0==strcmp("parallell",splitprocbuff[*curcmd][1])) || (0==strcmp("p",splitprocbuff[*curcmd][1])))
				(*curcmd)++;
			else if ((0==strcmp("sequential",splitprocbuff[*curcmd][1])) || (0==strcmp("s",splitprocbuff[*curcmd][1])))
			{
				parmodewait(waitnum);
				flag = 'S';
				(*curcmd)++;
				printf("Mode Changed To Sequential Mode\n");
			}
			else
			{			
				printf("ERROR: Unrecognized Mode Command\n");
				(*curcmd)++;
			}
		}
		else /* We have something that we are going to interpret as a program to executed */
		{
			pid_t id = fork();
			if (id==0)
			{
				if (execv(splitprocbuff[*curcmd][0],splitprocbuff[*curcmd])<0)
				{
					fprintf(stderr,"ERROR: Failed to Execute File: %s\n", strerror(errno));
					exit(0);		
				}
			}
			else if (id >0)
			{
				waitnum++;
			}
			else 
				fprintf(stderr,"Failure to Fork: %s\n", strerror(errno));
			
			(*curcmd)++;
		}

	}
	return flag;
}





int main(int argc, char **argv)
{
	struct rusage usage;
	struct timeval kstart, kend, ustart, uend;

  	getrusage(RUSAGE_SELF, &usage);
 	kstart = usage.ru_stime;
	ustart = usage.ru_utime;


	char * prompt = "comence!> ";
	printf("%s", prompt);
	fflush(stdout);

	char buffer[1024];
	
	int flag = 'Q';
	/* There are 5 flags, S: Run (Continue to Run) this line in sequential mode, P: Run (Continue to Run) this line in 		parellel mode, Q: Ready for next line Sequential mode, L: Ready for next line parrellel mode E: Exit program */
	
	char ** processedbuffer;
	char * splitprocbuff[1025][1025];

	while ((fgets(buffer, 1024, stdin) != NULL) && ('Q'==flag || 'L' ==flag)) /* This reads and takes care of an entire line of input*/
	{
			

		/* processed is tokenized by ; so it is now an array of char *'s, it is NULL terminated*/
		processedbuffer = *processbuffer(buffer);
		int i =0;

		

		/*this tokenizes by whitespace and removes all blank commands*/
		splitprocbuff[1025][1025];
		char * temp;	
		i = 0;
		for(;NULL!=processedbuffer[i];i++)
		{
			temp=strtok(processedbuffer[i]," \t");
			int k = 1;	
			splitprocbuff[i][0]=temp;	
			for(;NULL != temp; k++)
			{
				temp = strtok(NULL," \t");
				splitprocbuff[i][k]=temp;
			}	
		}
		splitprocbuff[i][0]=NULL;
	


		int x = 0;
		int * currentcommand = &x;

		if('Q'== flag)
			flag='S';
		if('L' == flag)
			flag ='P';
		
		while ('P'== flag || 'S' == flag) /* the current line of input will be read */
		{	
		
			while('S' == flag)
			{
	
				flag=seqmode(splitprocbuff,currentcommand);
			}


			while ('P'==flag)
			{
				flag=parmode(splitprocbuff,currentcommand);
			}
		}
						

	
		if ('Q'==flag || 'L' ==flag) /* we are going to read the next line of input */
		{
			printf("%s", prompt);
			fflush(stdout);
		}

	}


  	getrusage(RUSAGE_SELF, &usage);
 	kend = usage.ru_stime;
	uend = usage.ru_utime;
	
	long int kernel = kend.tv_sec-kstart.tv_sec;
	long int user = uend.tv_sec-ustart.tv_sec;

	/* unfortuntly this part is not working and Im not sure how to make it work...*/
	printf("Kernel time used was: %lu Seconds\n",kernel);
	printf("User time used was: %lu Seconds\n",user);
	
	free(processedbuffer);

}



