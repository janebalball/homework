
#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>		// For memset()
#include	<sys/types.h>
#include	<sys/stat.h>		// For stat()
#include	<unistd.h>
#include	<fcntl.h>		// For open()
#include	<sys/socket.h>		// For socket()
#include	<netinet/in.h>		// For INADDR_ANY
#include	<signal.h>		// For sigaction
#include	<wait.h>		// For WNOHANG



#include	"headers.h"

//  PURPOSE:  To keep track of the current number of client handling children
//	that exist.
int 	numChildren	= 0;


//  PURPOSE:  To acknowledge the ending of child processes so they don't
//	stay zombies.  Uses a while-loop to get one or more processes that
//	have ended, and decrements 'numChildren'.  Ignores 'sig'.
//	No return value.
void	childHandler	(int	sig)
{
  //  I.  Application validity check:

  //  II.  'wait()' for finished child(ren):
  //  YOUR CODE HERE
	pid_t pid;
	int status;
	while((pid = waitpid(-1,&status,WNOHANG)>0))
	{
		numChildren--;
		if(WIFEXITED(status))
		{
			if(WEXITSTATUS(status)==EXIT_SUCCESS) 
				printf("Process %d succeeded.\n", pid);
			else 
				printf("Process %d failed.\n", pid);
		}
		else 
			printf("Process %d crushed.\n", pid);
	}
  //  III.  Finished:
}


//  PURPOSE:  To install 'childHandler()' as the 'SIGCHLD' handler.  No
//	parameters.  No return value.
void	installChildHandler	()
{
  //  I.  Application validity check:

  //  II.  Install handler:
  //  YOUR CODE HERE
	struct sigaction act ;
	memset(&act, '\0' ,sizeof(struct sigaction));
	act.sa_handler = childHandler;
	act.sa_flags = SA_NOCLDSTOP | SA_RESTART;
	sigaction(SIGCHLD, &act, NULL);

  //  III.  Finished:
}


//  PURPOSE:  To note that signal 'SIGCHLD' is to be ignored.  No parameters.
//	No return value.
void	ignoreSigChld	()
{
  //  I.  Application validity check:

  //  II.  Ignore SIGCHLD:
  //  YOUR CODE HERE
  struct sigaction act;
  memset(&act, '\0' , sizeof(struct sigaction));
  act.sa_handler = SIG_IGN;
  sigaction(SIGCHLD, &act,NULL);
  
  //  III.  Finished:
}


//  PURPOSE:  To return a port number to monopolize, either from the
//	'argv[1]' (if 'argc' > 1), or by asking the user (otherwise).
int	obtainPortNumber	(int	argc, char*	argv[])
{
  //  I.  Application validity check:

  //  II.  Obtain port number:
  int	portNum;

  if  (argc > 1)
  {
    //  II.A.  Attempt to obtain port number from 'argv[1]':
    portNum = strtol(argv[1],NULL,10);
  }
  else
  {
    //  II.B.  Attempt to obtain port number from user:
    char	numText[NUM_TEXT_LEN];

    do
    {
      printf("Port number (%d-%d)? ",LO_LEGAL_PORT_NUM,HI_LEGAL_PORT_NUM);
      fgets(numText,NUM_TEXT_LEN,stdin);
      portNum = strtol(numText,NULL,10);
    }
    while  ((portNum < LO_LEGAL_PORT_NUM) || (portNum > HI_LEGAL_PORT_NUM));

  }

  //  III.  Finished:
  return(portNum);
}


//  PURPOSE:  To return a socket for listening for clients if obtained one from
//	OS, or to send an appropriate error msg to 'stderr' and return -1
//	otherwise.  Sets '*portPtr' to the number of the port to use.
//	No parameters.  No parameters.
int 	createListeningSocket	(int	argc, char*	argv[], int*	portPtr)
{
  //  I.  Applicability validity check:

  //  II.  Create server socket:
  int socketDescriptor = ERROR_FD;
  //  YOUR CODE HERE
	*portPtr = obtainPortNumber(argc, argv);
	socketDescriptor = socket( AF_INET, SOCK_STREAM, 0 );
	if(socketDescriptor != -1)
	{
		struct sockaddr_in socketInfo;
		memset(&socketInfo, '\0', sizeof(socketInfo));
		socketInfo.sin_family = AF_INET;
		socketInfo.sin_port = ntohs(*portPtr);
		socketInfo.sin_addr.s_addr = INADDR_ANY;
		bind(socketDescriptor, (struct sockaddr*) &socketInfo, sizeof(socketInfo));
		listen(socketDescriptor, MAX_NUM_QUEUING_CLIENTS);
	}
	else
	{
		fprintf(stderr, "Cannot create listening socket!\n");
		return(ERROR_FD);
	}
  //  III.  Finished:
  return(socketDescriptor);
}


//  PURPOSE:  To attempt to obtain the filename and file size from the client
//	via 'clientDescriptor' and to put them into 'filename' and
//	'*filesizePtr' respectively.  'maxFilenameLen' tells the length of the
//	'filename' char array.  Returns '1' if successful or '0' otherwise.
int	didReadNameAndSize
			(int		clientDescriptor,
			 char*		filename,
			 int		maxFilenameLen,
			 unsigned int*	filesizePtr
			)
{
  //printf(" filename: %s\n",  filename);
  //printf(" maxFilenameLen: %d\n",  maxFilenameLen);
  //printf(" filesizePtr: %d\n",  *filesizePtr);

  //  I.  Application validity check:

  //  II.  Read filename and file size:
  //  YOUR CODE HERE
  unsigned int numBytes;
  
  	numBytes = read(clientDescriptor, filename, FILENAME_LEN);
	printf("filename: %s.\n", filename);
	if (numBytes < 0)
	{
		fprintf(stderr, "Could not read filename.\n");
		return(0);
	}

	unsigned int len;
	read(clientDescriptor, (void *)&len, sizeof(unsigned int));
	//len = (((len)&0xff)<<24) + (((len>>8)&0xff)<<16) + (((len>>16)&0xff)<<8) + (((len>>24)&0xff));
	printf("filesizePtr: %d.\n", len);
	
	*filesizePtr = ntohl(len);
    //off_t fsize = lseek(clientDescriptor, 0, SEEK_END);
    //	printf("filesizePtr: %d.\n", *filesizePtr);

  //  III.  Finished:
  return(1);
}


//  PURPOSE:  To do the work of handling the client with whom socket descriptor
//	'clientDescriptor' communicates.  Returns 'EXIT_SUCCESS' on success or
//	'EXIT_FAILURE' otherwise.
int	serveClient	(int		clientDescriptor,
			 const char*	filename,
			 unsigned int	fileSize
			)
{

  //  I.  Application validity check:

  //  II.  Serve client:
  //  II.A.  Create names of temporary files:
  //  YOUR CODE HERE
	char buffer_inFile[LINE_LEN];
	
	char inFilename[FILENAME_LEN];
	char psFilename[FILENAME_LEN];
	char pdfFilename[FILENAME_LEN];

	unsigned int size_PDF_h;
	unsigned int size_PDF_n;
	
	int write_status_inFile;
	int write_status_pdf;
	int write_status_pdf_size;
	
	pid_t pid = getpid();
	
	snprintf ( inFilename, FILENAME_LEN, "%s%d%s", TEMP_FILENAME_PREFIX, pid, TEXT_EXT);
	snprintf ( psFilename, FILENAME_LEN, "%s%d%s", TEMP_FILENAME_PREFIX, pid, POSTSCRIPT_EXT);
	snprintf ( pdfFilename, FILENAME_LEN, "%s%d%s", TEMP_FILENAME_PREFIX, pid, PDF_EXT);

	printf("%s, %s, %s\n", inFilename, psFilename, pdfFilename);

	
	int fd_inFile = open(inFilename,O_WRONLY | O_TRUNC | O_CREAT, 0660);
	if(fd_inFile < 0)
	{
		fprintf(stderr, "Failed to create .txt temporary file!\n");
		return(EXIT_FAILURE);
	}
	
	//char ch;
    int num = 0;
	//fcntl(clientDescriptor, F_SETFL, O_NONBLOCK);
	//while( (numBytes_inFile = read(clientDescriptor, buffer_inFile, fileSize)) >0)
	read(clientDescriptor, (void *)buffer_inFile, fileSize);
	write_status_inFile = write(fd_inFile, buffer_inFile, fileSize);
	
	//if(write_status_inFile < 0)
	//{
	//	fprintf(stderr, "Failed to write client files to .txt temporary file!\n");
	//	return(EXIT_FAILURE);
	//}
	
    if  ( fork() == 0 )
	{
		printf("txt to ps...");
		execl("/usr/bin/enscript", "enscript", "-B", inFilename, "-p", psFilename, "-q", NULL);		
	}
	else
		wait(NULL);
	
	if  ( fork() == 0 )
	{
		printf("ps to pdf...");
		execl("/usr/bin/ps2pdf12", "ps2pdf12", psFilename, pdfFilename, NULL);
	}		
	else
		wait(NULL);
	
	int fd_pdf = open(pdfFilename, O_RDONLY, 0);
	if(fd_pdf < 0)
	{
		fprintf(stderr, "Failed to open PDF temporary file!\n");
		return(EXIT_FAILURE);
	}
	
	struct stat pdfStat;
	fstat(fd_pdf, &pdfStat);
	size_PDF_h = pdfStat.st_size;
	size_PDF_n = htonl(size_PDF_h);
	
	char buffer_PDF[size_PDF_h];
    read(fd_pdf, buffer_PDF, size_PDF_h);
	write_status_pdf_size = write(clientDescriptor, &size_PDF_n, sizeof(unsigned int));
	write_status_pdf = write(clientDescriptor, buffer_PDF, size_PDF_h);

	if(write_status_pdf <0)
	{
		fprintf(stderr, "Failed to write PDF file to client!\n");
		return(EXIT_FAILURE);
	}

	if(write_status_pdf_size <0)
	{
		fprintf(stderr, "Failed to write PDF file size to client!\n");
		return(EXIT_FAILURE);
	}

	close(fd_inFile);
	close(fd_pdf);
	
	remove(inFilename);
	remove(psFilename);
	//remove(pdfFilename);


  //  III.  Finished:
  return(EXIT_SUCCESS);
}


//  PURPOSE:  To do the work of the server: waiting for clients, fork()-ing
//	child processes to handle them, and going back to wait for more
//	clients.  No return value.
void	doServer	(int	socketDescriptor)
{
  //  I.  Application validity check:

  //  II.  Serve clients:
  //  II.A.  Each iteration serves one client:
  char	text[LINE_LEN];
  int	i;

  printf(" NUM_SERVER_ITERS: %d\n",  NUM_SERVER_ITERS);
  printf(" socketDescriptor: %d\n",  socketDescriptor);
  printf(" LINE_LEN: %d\n",  LINE_LEN);
  for  (i = 0;  i < NUM_SERVER_ITERS;  i++)
  {
printf("wait ...\n");

 
    //struct sockaddr_in their_addr; /* connector's address information */   
    //int sin_size;     
    //sin_size = sizeof(struct sockaddr_in); 

    int	clientDescriptor = accept(socketDescriptor, NULL, NULL); // <-- CHANGE THAT INTEGER!
	printf("accept ...\n");
    char		filename[FILENAME_LEN];
    unsigned int	fileSize;
    pid_t		childPid;

    if  (clientDescriptor < 0)
	  continue;
  
    if  (!didReadNameAndSize(clientDescriptor,filename,FILENAME_LEN,&fileSize))
      continue;

    childPid	= fork(); // <-- CHANGE THAT INTEGER!

    if  (childPid < 0)
    {
      continue;
    }
    else
    if  (childPid == 0)
    {

      ignoreSigChld();

      close(socketDescriptor);

      int status = serveClient(clientDescriptor,filename,fileSize);

      close(clientDescriptor);
      exit(status);

    }
    else
    {
      numChildren++;
      printf("%d: %s (%d bytes) (process %d) ...\n",i,filename,fileSize,childPid
	    );
      close(clientDescriptor);
    }


  }

  //  II.B.  Wait for all children to finish:
  while  (numChildren > 0)
    sleep(1);

  //  III.  Finished:
}


int	main	(int	argc,
		 char*	argv[]
		)
{
  //  I.  Application validity check:

  //  II.  Serve clients
  //  II.A.  Get socket file descriptor:
  int port;
  int socketDescriptor = createListeningSocket(argc,argv,&port);
printf("socketDescriptor %d ...\n", socketDescriptor);

  if  (socketDescriptor == -1)
    return(EXIT_FAILURE);

  //  II.B.  Set up SIGCHLD handler:
  installChildHandler();
printf("installChildHandler ...\n");


  //  II.C.  Handle clients:
  doServer(socketDescriptor);
printf("doServer ...\n");

  //  III.  Will get here if 'doServer()' quits:
  return(EXIT_SUCCESS);
}

