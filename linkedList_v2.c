
/*
*******************************************
* Program Name  : linkedList.c             *
* Author		: Sandesh Sharma.         *
* Date			: 4/8/2021                *
*******************************************
*/
// Program Description
// ------------------------------------------------------------------------
// | A simple implementation of linked list structure 		  			  |
// | to make an employee management system in a medium scale organization |
// | with an added functionality of error logging and reading those logs  |
// | over a TCP server. (Demoed on IPv4 Loopback);						  |
// ------------------------------------------------------------------------


#if defined(_WIN32) // If on Windows enviroment
#ifndef _WIN32_WINNT // if not defined
#define _WIN32_WINNT 0x0600 //define it
#endif
#include <winsock2.h> // Header firles for windows
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else // Linux/UNIX enviroment
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#endif

//Define Pre-processor macros for portability.
#if defined(_WIN32) // Macros for Windows 
#define ISVALIDSOCKET(s) ((s) != INVALID_SOCKET)
#define CLOSESOCKET(s) closesocket(s)
#define GETSOCKETERRNO() (WSAGetLastError())

#else // Macors for UNIX

#define ISVALIDSOCKET(s) ((s) >= 0)
#define CLOSESOCKET(s) close(s)
#define SOCKET int
#define GETSOCKETERRNO() (errno)

#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>







struct Person{ //Person structure..

	int id;
	char fName[30];
	char lName[40];
	int  age;
	char emailAddress[50];
	unsigned long int phone;
		
	struct Person * next;
};



typedef struct errorStruct{

	int errYear;
	int errMonth;
	int errDoM;
	int errDoW;
	int errToD;
	int errM;
	int errS;
	bool valAssocFlag;
	int valueAssoc;
	char errMessage[256];
	char errTimeZone[4];
	

}errStruct;



static clock_t begin,end;

static errStruct sessionErrorInit;

static FILE * logFile = NULL;

static int changesMade = 0,fileOverwrittenFlag= 0;

static int totalNodes;


int recursiveSearch(struct  Person ** hNode, int nodeCount,int searchVal);

void throwError(int errorCode , int lineNo); //Prints an error message and line number.

void displayMessage(int msgCode,int val); //Prints appropriate message according to predefined message code.

int insertNode(struct Person ** headNode , struct Person newNode); // inserts node and returns 0 (can be used later for error codes)

int printList(struct Person * headNode, int printList); // Returns the total number of items

int searchList(struct Person * headN, int searchVal, int searchFlag); //searches the list for a value. If found, returns the position (starting from 1), else 0; (Linear Search, O(n) = n);

int deleteNode (struct Person ** firstNode, int delVal); // deletes a list and returns 1 if sucessfully deletes, 0 if not found;

struct Person * deleteList(struct Person ** entryNode); // deletes the entire list except for the head. Set's head to NULL;

void printInfo(struct Person Node); //  Prints a node information

int overwriteFile(struct Person ** newHead, char fileName[20]); // Overwrites the files with new information
 
static bool self_init(); //Initializes log file and errStruct, returns false on failure

int netLog(); // Reads the log and passes it over a tcp network. (Currently set to localhost:8080);

void readLog(); // Reads the log and prints the log on the standard console screen (calls printErrLog() function)

void printErrLog(errStruct a); // Reades the errStruct structure and prints the inforamtion in a human readable manner.

static bool initErrorStruct(int year, int month, int dOfMonth, int dOfWeek);

SOCKET InitializeSocket();





SOCKET InitializeSocket(){


		//printf("Configuring local address...\n");
		displayMessage(10,0);
		struct addrinfo hints; // declar a addrinfo structure
	 
		memset(&hints, 0, sizeof(hints)); // Fill it will 0 values	 
		hints.ai_family = AF_INET; // IPv4 (AF_INET6 for IPv6);
		hints.ai_socktype = SOCK_STREAM; // for TCP connection (SOCK_DGRAM for UDP).
		hints.ai_flags = AI_PASSIVE; // Any network interface available. Has to be set before callind getaddrinfo. 
		struct addrinfo *bind_address; // pointer to an addrinfo struct to save the values reteurned by getaddrinfo();

		getaddrinfo(0, "8080", &hints, &bind_address); // First param NULL, 2nd: Port to listen on, hints: What we're looking for
															// bindaddress:: return buffer.
		//printf("Creating socket...\n");
		displayMessage(11,0);
		SOCKET socket_listen; // Create a socket.. In Wondows, it's WINSOCK type. On UNIX enviroment , it's an unsigned int (file descriptor)

					//socket: parameters  //socket family, socket type and socket protocol
					//returns -ve int on UNIX enviroment and INVALID_SOCKET error on Windows enviroment;
		socket_listen = socket(bind_address->ai_family,bind_address->ai_socktype, bind_address->ai_protocol);
		if (!ISVALIDSOCKET(socket_listen)) { // If invalid, get errorNo , WSAGetLastError() on Windows, errno on UNIX enviroment;
				return INVALID_SOCKET;		
		}
		
		//("Binding socket to local address...\n");


		// Binding the socket too a local address, 
		// bind() returns 0 on success and non-zero on failure.
		if (bind(socket_listen,bind_address->ai_addr, bind_address->ai_addrlen)) {  //  If non-zero, throw error.
				//fprintf(stderr, "bind() failed. (%d)\n", GETSOCKETERRNO());
				freeaddrinfo(bind_address); // Now that we've binded the socket, free the memory for addrinfo;
				return INVALID_SOCKET;
		}
		freeaddrinfo(bind_address); // Now that we've binded the socket, free the memory for addrinfo;
		//displayMessage(14,8080);
		//printf("Listening...\n"); //
	
		printf("Waiting for connection...\n");
		return socket_listen;
}


void printErrLog( errStruct a){


	if(a.valAssocFlag){
		printf("\n[ERR LOG] [ %d/%d/%d  %d:%d:%d ] %s %d",a.errDoM,a.errMonth,a.errYear,a.errToD,a.errM,a.errS,a.errMessage,a.valueAssoc);
	}else{

		printf("\n[ERR LOG] [ %d/%d/%d  %d:%d:%d ] %s",a.errDoM,a.errMonth,a.errYear,a.errToD,a.errM,a.errS,a.errMessage);
	}
	


}

static bool initErrorStruct(int year, int month, int dOfMonth, int dOfWeek){

	bool retFlag = false;

	sessionErrorInit.errYear = year;
	sessionErrorInit.errMonth = month;
	sessionErrorInit.errDoM = dOfMonth;
	sessionErrorInit.errDoW = dOfWeek;
	sessionErrorInit.valAssocFlag = false;

	strncpy(sessionErrorInit.errTimeZone,"UTC\0",4);

	retFlag = true;
	return retFlag;

}

void readLog(){

	errStruct temp;
	fseek(logFile,0,SEEK_SET);
	if(logFile != NULL){

		while(!feof(logFile)){
			fread(&temp,sizeof(errStruct),1,logFile);
			printErrLog(temp);
		}
	}
}


int netLog(){

	int retFlag = 0,bytes_sent,flag=0;
	char line[256],buffer[20];
	if(logFile == NULL){

		displayMessage(9,0);

	}else{



		errStruct  temp ;


		SOCKET errSock = InitializeSocket();

		if(ISVALIDSOCKET(errSock)){

			displayMessage(14,8080);

			if (listen(errSock, 10) < 0) { // Listen on that socket. 2nd parameter (10) is the max number that the server can queue up.
													 // If the queue is full, any additional incoming requests are rejected until the queue is free.
													// 
				fprintf(stderr, "listen() failed. (%d)\n", GETSOCKETERRNO());
				return 1;
			}
				//printf("Waiting for connection...\n");

			struct sockaddr_storage client_address; //
			socklen_t client_len = sizeof(client_address);
			SOCKET socket_client = accept(errSock,(struct sockaddr*) &client_address, &client_len);

			if(ISVALIDSOCKET(socket_client)){

				char response[1024];
				char header[] = "\r\nHTTP/1.1 200 OK\r\nConnection: close\r\nContent-Type: text/plain\r\n\r\n";

				displayMessage(15,8080);//Client Connected;

				retFlag = 1;
				fseek(logFile,0,SEEK_SET);

				while(!feof(logFile)){

					fread(&temp,sizeof(errStruct),1,logFile);
					//strncat(response+sizeof(header),temp.errMessage,sizeof(temp.errMessage));
						
					if(flag == 0){
						strncpy(response,header,strlen(header));
						strncat(response,temp.errMessage,sizeof(temp.errMessage));
						flag=1;
					}else{

						strncpy(response,temp.errMessage,sizeof(temp.errMessage));

					}

					if(temp.valAssocFlag == 1){

						itoa(temp.valueAssoc,buffer,10);
						strncat(response,buffer,sizeof(buffer));
					}

					strcat(response,"\r\n");
						//strncat(response,'\0',1);
					bytes_sent = send(socket_client,response, strlen(response), 0);


					printf("\nSent %d of %d bytes.\n", bytes_sent, (int)strlen(response));
					memset(response,0,sizeof(response));
					memset(buffer,0,sizeof(buffer));
						//printf("\n%s",temp->errMessage);
						//printErrLog(temp);
						
				}

					
					
				
				printf("\nClosing connection...\n");
				CLOSESOCKET(socket_client);

				printf("\nClosing listening socket...\n");
				CLOSESOCKET(errSock);
				#if defined(_WIN32)
					WSACleanup();
				#endif
				printf("Finished.\n");
				retFlag = 1;
			}else{

				displayMessage(12,GETSOCKETERRNO());
			}
		}
	


	}


	return retFlag;


}




static bool self_init(){


	logFile = fopen("log.bin","ab+");



	if(logFile == NULL){

		return false;

	}else{

		time_t t = time(NULL);
		struct tm *tmp = gmtime(&t);
		

		if(initErrorStruct((tmp->tm_year)+1900,(tmp->tm_mon)+1,tmp->tm_mday,tmp->tm_wday)){

			return true;

		}else{

			return false;

		}

		
		

	}

	
}



void done(){

	if(fileOverwrittenFlag == 1){

		displayMessage(8,0);
	}else{

		displayMessage(7,0);
	}

	displayMessage(6,0);

	fclose(logFile);

}


int overwriteFile(struct Person ** newHead, char fileName[20]){


	FILE * overWrite = fopen(fileName,"wb");
	printf("\nOverWrite FileName : %s\n",fileName);

	int retFlag = 0;

	if(overWrite != NULL && *newHead != NULL){
		fseek(overWrite,0,SEEK_SET);
		retFlag = 1;
		struct Person * walker = (struct Person*)malloc(sizeof(struct Person));
		walker = *newHead;
		//rewind(overWrite);


		while(walker != NULL){

			fwrite(walker,sizeof(struct Person),1,overWrite);
			printf("\nId : %d",walker->id);
			walker = walker->next;
		}

		deleteList(newHead);
		//displayMessage(8);
		fileOverwrittenFlag=1;
		//free(*newHead);
		fclose(overWrite);

	}else{

		throwError(4,__LINE__);
	}


	return retFlag;
}



// Prints information of the structure passed;
//***************************************************
void printInfo(struct Person Node){



	printf("\n*************************************\n*\t\tEmployee Info\t\t*\n******************************************\n");
	printf("Emp. ID: %d\nFirst Name: %s\nLast Name: %s\nAge: %d\nE-Mail: %s\nPhone : %lu\n",Node.id,Node.fName,Node.lName,Node.age,Node.emailAddress,Node.phone);


}



// Delete the entire list-->Set head=NUll and return head;
//**********************************************************************
struct Person * deleteList(struct Person ** entryNode){


	if(*entryNode == NULL){ //If head is NULL, throws and error and returns the head..

		throwError(-1,__LINE__);
	}else{ //delete every node except head.. Set head to NULL


		struct Person* current = *entryNode;
		struct Person* next;
		while(current != NULL){
			
			next = current->next;
			free(current);
			current = next;

		}

		changesMade = 1;
	}

	*entryNode = NULL;
	return *entryNode;
}


void displayMessage(int msgCode, int val){



	time_t t = time(NULL);
	struct tm *tmp = gmtime(&t);
		
	static int a =0;
	
	

	errStruct tempLog;


	tempLog.errYear = sessionErrorInit.errYear;
	tempLog.errMonth = sessionErrorInit.errMonth;
	tempLog.errDoM = sessionErrorInit.errDoM;
	tempLog.errDoW = sessionErrorInit.errDoW;

	tempLog.errToD = tmp->tm_hour;
	tempLog.errM = tmp->tm_min;
	tempLog.errS = tmp->tm_sec;
	tempLog.valAssocFlag = 0;

	switch(msgCode){



		case(1):

			printf("\nValue %d Deleted Sucessfully...",val);
			tempLog.valAssocFlag = true;
			tempLog.valueAssoc = val;

			strcpy(tempLog.errMessage,"Value Deleted :");
			break;

		case(2):

			printf("\nList Sucessfully Deleted..\n");
			
			strcpy(tempLog.errMessage,"\nList Sucessfully Deleted..");
			
			break;

		case(3):

			printf("\nValue %d not found..\n\n",val);
			tempLog.valAssocFlag = true;
			tempLog.valueAssoc = val;

			strcpy(tempLog.errMessage,"Value Not Found :");
			
			break;

		case(4):

			printf("\nThe list is empty..\n\n");
			strcpy(tempLog.errMessage,"The list is empty..");
			
			break;
		case(5):

			printf("\nFile Sucessfully Overwritten\n");
			strcpy(tempLog.errMessage,"File Sucessfully Overwritten");
		
			break;

		case(6):

			printf("\n\nNow exiting the program....\n\n");
			strcpy(tempLog.errMessage,"Now exiting the program....");
		
			break;
		
		case(7):

			printf("\n[EXIT MESSAGE] : No changes were made to the file.\n");
			strcpy(tempLog.errMessage,"[EXIT MESSAGE] : No changes were made to the file.");
			
			break;

		case(8):

			printf("\n[EXIT MESSAGE] : Changes were made to the file\n");
			strcpy(tempLog.errMessage,"[EXIT MESSAGE] : Changes were made to the file");
		
			break;

		case(9):

			printf("\n[ERR] : Program Failed to Initialize.\n");
			strcpy(tempLog.errMessage,"[ERR] : Program Failed to Initialize.");
			break;
		case(10):
			printf("\nConfiguring local address...\n");
			strcpy(tempLog.errMessage,"[MSG] : Configuring local address...");
			break;
		case(11):
			printf("\nCreating socket...\n");
			strcpy(tempLog.errMessage,"[MSG] : Creating socket...");
			break;
		case(12):
			printf("\nSocket Initialization failed (%d)..\n\n",val);
			tempLog.valAssocFlag = true;
			tempLog.valueAssoc = val;
			strcpy(tempLog.errMessage,"Socket Initialization Failed. ");
			break;
		case(13):
			printf("\nBinding socket to local address...\n");
			strcpy(tempLog.errMessage,"[MSG] : Binding socket to local address...");
			break;
		case(14):
			tempLog.valAssocFlag = true;
			tempLog.valueAssoc = val;
			printf("\nSocket Listening on Port %d [Awaiting Connection]",val);
			strcpy(tempLog.errMessage,"[MSG]: Socket Listening [Awaiting Connection] on Port ");
			break;
		case(15):
			tempLog.valAssocFlag = true;
			tempLog.valueAssoc = val;
			printf("\nClient Connected on Port %d\n",val);
			strcpy(tempLog.errMessage,"[MSG] : Client Connected on Port : ");
			break;
			
	}

	fwrite(&tempLog,sizeof(tempLog),1,logFile);


}


int deleteNode(struct Person ** firstNode, int delVal){


	int retFlag = -1,exist,i=0;

	if(*firstNode == NULL){ // If head is NULL (Lists Empty), throw an error and return;

		throwError(-1,__LINE__); 

	}else{

		printf("\nNode %d about to be deleted..\n");

		struct Person *walker = *firstNode;

		exist = searchList(*firstNode,delVal,1); //Get the poaition of the value to be deleted. returns -1 if not found..
		if(exist >= 0){
			changesMade = 1;
			struct Person * temp = (struct Person*)malloc(sizeof(struct Person));
			retFlag = 1;
			if(exist == 0){ //If the item is at position 1, i.e the head, we need to change head to the current head's next

				temp = *firstNode;
				*firstNode = (*firstNode)->next;
				free(temp);
			
			
			}else{
			
				//Loop to go through the list until reached the desired location i.e 1 step before the index of value to be deleted.

				for(i=0;i<exist-1;i++){ 

					walker=walker->next;
				}

				
				
				temp = walker->next; // set temp = node to be deleted.
				walker->next = walker->next->next; // current node's next now points to it's next->next , i.e temp->next
				free(temp); //free temp 
			}

			displayMessage(1,delVal);
			changesMade = 1;
			
			//printf("\n\nValue %d Deleted Successfully..\n\n",delVal);
		}else{

			displayMessage(3,delVal);
		}

	}
	return retFlag;

}



int searchList(struct Person * headN,int searchVal, int searchFlag){

	int counter=0,foundFlag=-1;
	if(headN == NULL){

		if(searchFlag == 1){
			throwError(-1,__LINE__);
		}else{
			foundFlag = -2;
		}
	
	}else{

		struct Person *walker = headN;
		struct Person per;


			
			

			while(walker != NULL){
				
				if(walker->id == searchVal){

					if(searchFlag == 1){
						foundFlag=counter;
						per.id = walker->id;
						strncpy(per.fName,walker->fName,sizeof(walker->fName));
						strncpy(per.lName,walker->lName,sizeof(walker->lName));
						per.age = walker->age;
						strncpy(per.emailAddress,walker->emailAddress,sizeof(walker->emailAddress));
						per.phone = walker->phone;

					
						printf("\nFound at Position %d...\n",counter);
					
						printInfo(per);

					}else{

						foundFlag = 1;
					}
	
					
					
					break;
				}
			
				walker = walker->next;

				counter++;


			}
			if(foundFlag == -1 && searchFlag == 1){

				displayMessage(3,searchVal);
				
			}

		}



	return foundFlag;
}

int printList(struct Person * headNode,int printFlag){


	int itemCounter=0;

	printf("\n\n***********************************************************************\n\n");

	if(headNode == NULL){

		displayMessage(4,0);

	}else{

		struct Person *walker = (struct Person *)malloc(sizeof(struct Person));
		walker = headNode;
	
		while(walker != NULL){

			if(printFlag == 1){

				printf("| %d - %s - %s - %d - %s - %lu \n\n",walker->id, walker->fName,walker->lName,walker->age,walker->emailAddress,walker->phone);
			}
			
			walker = walker->next;
			itemCounter++;
		}
	}

	if(printFlag == 1){
		printf("\n\nTOTAL ITEMS : %d\n\n",itemCounter);
		printf("\n\nTOTAL SIZE (BYTES) : %lu (%lu KB)\n\n",itemCounter*sizeof(struct Person),(itemCounter*sizeof(struct Person))/1024);
	}
	return itemCounter;
}

int insertNode(struct Person ** headNode, struct Person newNode){

	int i=1;
	int itemCounter=0;
	
	if(*headNode == NULL){ //If head is null, that means the list doesn't exist.. Add the first node (head);

		*headNode = (struct Person *)malloc(sizeof(struct Person));

		if(*headNode == NULL){
			throwError(3,__LINE__);		
		}else{
		

			(*headNode)->id=newNode.id;
			strncpy((*headNode)->fName,newNode.fName,sizeof(newNode.fName)/sizeof(char));
			strncpy((*headNode)->lName,newNode.fName,sizeof(newNode.lName)/sizeof(char));
			(*headNode)->age=newNode.age;
			strncpy((*headNode)->emailAddress,newNode.fName,sizeof(newNode.emailAddress)/sizeof(char));
			//strncpy((temp->fName),newNode.fName,sizeof(newNode.phone)/sizeof(char));
			(*headNode)->phone = newNode.phone;
			(*headNode)->next = NULL;
			//*headNode = temp;
			itemCounter++;
			changesMade=1;
			printf("\nNew Node Id : %d",(*headNode)->id);
		}
	
	}else{


		struct Person *walker = (struct Person*)malloc(sizeof(struct Person));
		walker= *headNode ;
		//*headNode;
		while(walker->next != NULL){
			walker=walker->next;
			itemCounter++;

		}

		walker->next = (struct Person *)malloc(sizeof(struct Person));

		walker->next->id=newNode.id;
		strncpy(walker->next->fName,newNode.fName,sizeof(newNode.fName)/sizeof(char));
		strncpy(walker->next->lName,newNode.lName,sizeof(newNode.lName)/sizeof(char));
		walker->next->age=newNode.age;
		strncpy(walker->next->emailAddress,newNode.emailAddress,sizeof(newNode.emailAddress)/sizeof(char));
		walker->next->phone = newNode.phone;
	
		printf("\nNew Node Id : %d",walker->next->id);
		walker->next->next = NULL;
		itemCounter++;
		changesMade=1; // changes were made to our data..
		

	}

	return itemCounter;
}


void throwError(int errorCode, int lineNo){



	printf("\n[ERROR before Line : %d] errCode: %d\nerrMsg : ",lineNo,errorCode);

	switch(errorCode){


		case( 1):

			printf("\nInvalid Number of arguments passed..\n\n");
			break;
		case(2):

			printf("\nCould not open file..\n");
			break;
		
		case(3):

			printf("\nMemory Allocation Failed..");
			break;
		case(4):
			printf("\n\n[OVERWRITE ERROR] FILE could not be open...\n");
			break;
		case(5):
			printf("\n\n[READIN ERROR] DATA in unstructured.\n");

	}
	
	
}
	


int main(int argc, char *argv[]){

			unsigned long int a=0,counter=0;
			int i;
			struct Person newPerson;


			FILE* fp = NULL;
			char fileName[20], userAns='n', userAns2='n',garbageChar,userAns3 = 'n';
			struct Person * head = NULL;

			begin = clock();
			if(argc != 2 || !self_init() ){

				throwError(1,__LINE__);
				return -1;

			}else{


				fp = fopen(argv[1],"rb");


				if(fp == NULL){

					throwError(2,__LINE__);
					

					
				}else{

					

					#if defined(_WIN32)
						WSADATA d;
						if (WSAStartup(MAKEWORD(2, 2), &d)) { // On Windows, need to initialize WinSock. (2,2) means version 2.2
							fprintf(stderr, "Failed to initialize.\n");
						return 1;
						}
					#endif

						strncpy(fileName,argv[1],strlen(argv[1]));
						printf("\nFileName : %s\n\n",fileName);
						//rewind(fp);
						fseek(fp,0,SEEK_SET);


					while(!feof(fp)){


						fread(&newPerson,sizeof(struct Person),1,fp);
						printf("\nNode %d\n",newPerson.id);

						if(searchList(head,newPerson.id,0) ==  -2 || searchList(head,newPerson.id,0) != 1  ){
							insertNode(&head,newPerson);
							a++;

						}else{

							printf("\nValue %d already exists",newPerson.id);
						}
						
					}

					totalNodes = a;

					changesMade=0;
					fclose(fp); // Close the file immediately;




					//readFile(&head,&fp);
			
					
					searchList(head,24,1); // Flag 1 returns 1 if found.. any value except 1 returns the index/position
					searchList(head,243,1);

					printList(head,1); // 1 flag prints the total number of items and file size in bytes.
					
				
					deleteNode(&head,24);
					deleteNode(&head,32);
					
				
					printList(head,1);
					netLog();
					//readLog();	
					//netLog();

					
				}
				

					

				
				
			}

			//If any chages were made in this session (global boolean variable set to false by default. 
			//If any changes are made by any function, they'll set it to true"
			//, then only ask to overwrite confirmation , or else don't

			if(changesMade == 1){

				printf("\n\nDo you want to save changes to the file?: ");
				scanf("%c",&userAns);

				if(userAns == 'y' || userAns == 'Y'){

					do{
						printf("\nAre you sure? [ !!! WARNING !!! This change cannot be undone ] : ");
						scanf(" %c",&userAns2);
						if(userAns2 == 'y' || userAns2 == 'Y'){

							i = overwriteFile(&head,argv[1]);

							if(i == 0){

								printf("Do you want to go again?: ");
								scanf("%c",&userAns3);

							}else{

								userAns3 = 'n';
							}

						}else{

							
							deleteList(&head);
							userAns3 = 'n';
						}
					}while(userAns3 == 'y' || userAns3 == 'Y');
				
				}else{

					deleteList(&head);
				}

			}else{

				deleteList(&head);
			}


					#if defined(_WIN32)
						WSACleanup();
					#endif
			
		free(head);
		

		atexit(done);

		end = clock();

		printf("\nRun Time : %.2f",(float)(end-begin)/CLOCKS_PER_SEC);


		return 0;


}

