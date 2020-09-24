// tcptunnel2.cpp : Defines the entry point for the console application.

#include	"stdafx.h"
#include	<stdio.h>
#include	<windows.h>
#include	<winsock.h>
#include	<process.h>
#include    <stdlib.h>
#include	"tcptunnel.h"

#define		UDP_TIMEOUT		30		// 30 seconds

UDPTunnelServer::UDPTunnelServer(
	int cPort,char* ipAddr,int bePort)
{
	clientPort = cPort;
	strcpy(BackEndServerIPAddr, ipAddr);
	BackEndServerPort = bePort;
	
	WSADATA			Data;


	SOCKADDR_IN		recvSockAddr;
	SOCKADDR_IN		sendSockAddr;

	SOCKADDR_IN		backEndServerSockAddr;

	SOCKET			recvSocket;
	SOCKET			sendSocket;

	int				status;
	int				numrcv;
	char			buffer[MAXBUFLEN];

	/* initialize the Windows Socket DLL */
	status=WSAStartup(MAKEWORD(1, 1), &Data);
	if (status != 0)
		cerr << "ERROR: WSAStartup unsuccessful" << endl;

	memset(&recvSockAddr, 0, sizeof(recvSockAddr));
	recvSockAddr.sin_port=htons(clientPort);
	recvSockAddr.sin_family=AF_INET;
	recvSockAddr.sin_addr.s_addr=htonl(INADDR_ANY);

	memset(&sendSockAddr, 0, sizeof(sendSockAddr));
	sendSockAddr.sin_port=htons(0);
	sendSockAddr.sin_family=AF_INET;
	sendSockAddr.sin_addr.s_addr=htonl(INADDR_ANY);
		
	/* create a socket */
	recvSocket=socket(AF_INET, SOCK_DGRAM, 0);
	if (recvSocket == INVALID_SOCKET)
		cerr << "ERROR: socket unsuccessful" << endl;

	sendSocket=socket(AF_INET, SOCK_DGRAM, 0);
	if (sendSocket == INVALID_SOCKET)
		cerr << "ERROR: socket unsuccessful" << endl;
	
	
	/* associate the socket with the address */
	status=bind(recvSocket, 
		(LPSOCKADDR) &recvSockAddr,
		sizeof(recvSockAddr));
	if (status == SOCKET_ERROR)
		cerr << "ERROR: bind unsuccessful" << endl;

	status=bind(sendSocket, 
		(LPSOCKADDR) &sendSockAddr,
		sizeof(sendSockAddr));
	if (status == SOCKET_ERROR)
		cerr << "ERROR: bind unsuccessful" << endl;


	// Making BackEndServer Address information

	unsigned long destAddr = inet_addr(BackEndServerIPAddr);

	struct in_addr iaHost;
	LPHOSTENT lpHostEntry;

	struct in_addr *pinAddr;
	
	iaHost.s_addr = inet_addr(BackEndServerIPAddr);
	if (iaHost.s_addr == INADDR_NONE)
	{
		lpHostEntry = gethostbyname(BackEndServerIPAddr);

		if (lpHostEntry == NULL)
			throw SocketPanic(0,"ERROR: gethostbyname unsuccessful\n");

		pinAddr = (LPIN_ADDR)lpHostEntry->h_addr_list[0];
		destAddr = inet_addr(inet_ntoa(*pinAddr));

	}

	memcpy(&backEndServerSockAddr.sin_addr, &destAddr, sizeof(destAddr));
	backEndServerSockAddr.sin_port = htons(BackEndServerPort);
	backEndServerSockAddr.sin_family = AF_INET;

	while(1)
	{
		fd_set	bitmask;
		struct timeval timeout;

		timeout.tv_sec = UDP_TIMEOUT;
		timeout.tv_usec = 0;

		try{

			// Step.1
			memset(buffer,0,MAXBUFLEN);

			struct sockaddr_in returnAddr;
			int	returnAddrLen = sizeof(returnAddr);

			numrcv = recvfrom(
				recvSocket, buffer, MAXBUFLEN, 0, 
				(struct sockaddr *)&returnAddr,&returnAddrLen);

			if (numrcv == SOCKET_ERROR)
				throw SocketPanic(0,"ERROR: recvfrom unsuccessful\n");

			//Step.2
			int numsnt=sendto(
				sendSocket, buffer,	numrcv, 0,
				(LPSOCKADDR) &backEndServerSockAddr, 
				sizeof(backEndServerSockAddr));

			if (numsnt == SOCKET_ERROR)
				throw SocketPanic(0,"ERROR: sendto unsuccessful\n");


			memset(buffer,0,MAXBUFLEN);

			FD_ZERO(&bitmask);			
			FD_SET(sendSocket, &bitmask);
			
			if (select(0,&bitmask,NULL, NULL, &timeout) > 0)
			{
				//Step.3
				numrcv = recvfrom(
					sendSocket, buffer, MAXBUFLEN, 0, 
					NULL, NULL);

				if (numrcv == SOCKET_ERROR)
					throw SocketPanic(0,"ERROR: recvfrom unsuccessful\n");

				//Step.4	
				numsnt=sendto(
					recvSocket, buffer,	numrcv, 0,
					(struct sockaddr *)&returnAddr,returnAddrLen);


				if (numsnt == SOCKET_ERROR)
					throw SocketPanic(0,"ERROR: sendto unsuccessful\n");
			}
		}

		catch(SocketPanic socketPanic)
		{
			printf("%s\n",socketPanic.reason());
		}

	} /* while */
}




TCPTunnelServer::TCPTunnelServer(
	int cPort,char* ipAddr,int bePort)
{
	clientPort = cPort;
	strcpy(BackEndServerIPAddr, ipAddr);
	BackEndServerPort = bePort;
	
	WSADATA			Data;
	int status = WSAStartup(MAKEWORD(1,1), &Data);
	if (status != 0)
		throw SocketPanic(0,"ERROR:WSAStartup unsuccessful\n");

	SOCKADDR_IN		serverSockAddr;
	memset(&serverSockAddr, 0, sizeof(serverSockAddr));

	serverSockAddr.sin_port = htons(cPort);
	serverSockAddr.sin_family = AF_INET;
	serverSockAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	listenSocket=socket(AF_INET, SOCK_STREAM, 0);
	if(listenSocket == INVALID_SOCKET)
	{
		throw SocketPanic(0,"ERROR:socket unsuccessful in TCPTunnelServer\n");
	}

	status = bind(
		listenSocket, (LPSOCKADDR)&serverSockAddr, sizeof(serverSockAddr));
	if(status == SOCKET_ERROR)
	{
		throw SocketPanic(0,"ERROR:bind unsuccessful in TCPTunnelServer\n");
	}
	
	status = listen(listenSocket, 1);
	if(status == SOCKET_ERROR)
	{
		throw SocketPanic(0, "ERROR:listen unsuccessful in TCPTunnelServer\n");
	}
}


TCPTunnelServer::connect()
{
	unsigned long destAddr = inet_addr(BackEndServerIPAddr);

	struct in_addr iaHost;
	LPHOSTENT lpHostEntry;

	struct in_addr *pinAddr;
	
	iaHost.s_addr = inet_addr(BackEndServerIPAddr);
	if (iaHost.s_addr == INADDR_NONE)
	{
		lpHostEntry = gethostbyname(BackEndServerIPAddr);

		if (lpHostEntry == NULL)
			throw SocketPanic(0,"ERROR: gethostbyname unsuccessful\n");

		pinAddr = (LPIN_ADDR)lpHostEntry->h_addr_list[0];
		destAddr = inet_addr(inet_ntoa(*pinAddr));

	}

	SOCKADDR_IN destSockAddr;

	memcpy(&destSockAddr.sin_addr, &destAddr, sizeof(destAddr));
	destSockAddr.sin_port = htons(BackEndServerPort);
	destSockAddr.sin_family = AF_INET;

	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket == INVALID_SOCKET)
	{
		shutdown(serverSocket,2);
		closesocket(serverSocket);
		throw SocketPanic(0,"ERROR: socket unsuccessful\n");
	}

	printf("Trying to connect to %s \n",BackEndServerIPAddr);

	int status = ::connect(
		serverSocket, (LPSOCKADDR)&destSockAddr, sizeof(destSockAddr));

	if (status == SOCKET_ERROR)
	{
		cout << "Error number: " << GetLastError() << endl;

		shutdown(serverSocket,2);
		closesocket(serverSocket);

		shutdown(clientSocket,2);
		closesocket(clientSocket);
		
		throw SocketPanic(0,"ERROR: connect unsuccessful\n");
	}
}


unsigned int __stdcall _digTunnel(void *tcpTunnelServer)
{
	TCPTunnelServer* tc = (TCPTunnelServer *)tcpTunnelServer;
	
	try
	{
		tc->connect();
		tc->digTunnel();
		_endthreadex(0);
	}
	catch(SocketPanic socketPanic)
	{
		printf("%s\n",socketPanic.reason());
	}
	catch(ThreadPanic threadPanic)
	{
		printf("%s\n",threadPanic.reason());
	}
	
	return 0;
}


TCPTunnelServer::digTunnel()
{

	TCPTunnel tunnel = TCPTunnel(clientSocket, serverSocket);
	tunnel.start();
	printf("This TCPTunnel is be destructed\n");
}


TCPTunnelServer::start()
{
	SOCKADDR_IN		clientSockAddr;
	int				addrLen=sizeof(SOCKADDR_IN);
	
	while(1)
	{
		clientSocket 
			= accept(listenSocket,(LPSOCKADDR)&clientSockAddr, &addrLen);
		if (clientSocket == INVALID_SOCKET)
		{
			shutdown(clientSocket,2);
			closesocket(clientSocket);
			
			throw SocketPanic(0,"ERROR: accept unsuccessful\n");
		}

		unsigned threadID;

		HANDLE threadHandle
			= (HANDLE)_beginthreadex(NULL, 0, &_digTunnel, this, 0, &threadID); 

		if (threadHandle == 0)
		{
			shutdown(clientSocket,2);
			closesocket(clientSocket);
		}
	}
}


TCPTunnelServer::close()
{
	// revisit
}


TCPTunnel::TCPTunnel(SOCKET cs, SOCKET ss)
{
	clientSocket = cs;
	serverSocket = ss;
	lifeFlag = true;
}

unsigned int __stdcall _returnWay( void* tcpTunnel )
{
	TCPTunnel* tt = (TCPTunnel *)tcpTunnel;

	try
	{
		tt->returnWay();
		_endthreadex(0);
	}
	catch(SocketPanic socketPanic)
	{
		printf("%s\n",socketPanic.reason());
	}
	catch(ThreadPanic threadPanic)
	{
		printf("%s\n",threadPanic.reason());
	}
		
	return 0;
}


void
TCPTunnel::returnWay()
{
	while(lifeFlag)
	{
		// From BackEndServer to Client

		char recvData[MAXBUFLEN];

		int numberRecv = recv(serverSocket, recvData, MAXBUFLEN, 0);
		if ((numberRecv == SOCKET_ERROR) || (numberRecv == 0))
		{
			lifeFlag = false;
			break;
		}
		
		int numberServ = send(clientSocket, recvData, numberRecv, 0);
		if (numberServ == SOCKET_ERROR)
		{
			lifeFlag = false;
			break;
		}
	}

	// KCG shutdown to trigger other listening thread..to close since 
	// you are blocking on other thread.
      
	shutdown(serverSocket,2);
    shutdown(clientSocket,2);
    closesocket(serverSocket);
    closesocket(clientSocket);
    //KCG end

     printf("END  ServerSocket %d\n",serverSocket);	
	return;	
}

void
TCPTunnel::start()
{
	unsigned  threadID;

	HANDLE	tunnelThreadHandle
		= (HANDLE)_beginthreadex(NULL, 0, &_returnWay, this, 0, &threadID); 
	
	if (tunnelThreadHandle == 0)
	{
		shutdown(serverSocket,2);
		closesocket(serverSocket);

		shutdown(clientSocket,2);
		closesocket(clientSocket);
		throw ThreadPanic(0,"returnWay thread failed\n");
	}

	while(lifeFlag)
	{
		// From Client to BackEndServer
		
		char recvData[MAXBUFLEN];

		int numberRecv = recv(clientSocket, recvData, MAXBUFLEN, 0);
		if ((numberRecv == SOCKET_ERROR) || (numberRecv == 0))
		{	
			lifeFlag = false;
			break;
		}
		
		int numberSend = send(serverSocket, recvData, numberRecv, 0);
		if (numberSend == SOCKET_ERROR)
		{
			lifeFlag = false;
			break;
		}
	}

	//WaitForSingleObject( tunnelThreadHandle, INFINITE );
	//CloseHandle( tunnelThreadHandle );

	shutdown(serverSocket,2);
	closesocket(serverSocket);

	shutdown(clientSocket,2);
	closesocket(clientSocket);

	WaitForSingleObject( tunnelThreadHandle, INFINITE );
	CloseHandle( tunnelThreadHandle );

}

unsigned int __stdcall _udpMainThread(void *connectInfo)
{
	struct ConnectInfo *ci = (struct ConnectInfo *)connectInfo;

	UDPTunnelServer 
		udpTunnelServer(ci->clientPort,ci->backEndServerIPAddr,ci->serverPort);

	return(0);
}

unsigned int __stdcall _tcpMainThread(void *connectInfo)
{
	struct ConnectInfo *ci = (struct ConnectInfo *)connectInfo;

	TCPTunnelServer 
		tcpTunnelServer(ci->clientPort,ci->backEndServerIPAddr,ci->serverPort);
	try
	{
		tcpTunnelServer.start();
	}
	catch (SocketPanic socketPanic) 
	{
		printf("%s\n",socketPanic.reason());
	}

	tcpTunnelServer.close();

	return(0);
}


#if _NTS
DWORD ServiceThread(LPDWORD param)  // This is for NT Service
#else
int main(void) 
#endif
{
	FILE *f;
	char string[1000];
	int counter = 0;

	char buffer[MAX_PATH];

	try
	{
		DWORD len = GetCurrentDirectory(MAX_PATH, buffer);
		
		if (len == 0) 
		{
			throw FilePanic(0,"Could not get current Directory...\n");
		}

	
		char configFileName[MAX_PATH];
		strcpy(configFileName, buffer);
		strcat(configFileName, "\\tcptunnel.cfg");


		f = fopen(configFileName,"r");
		if (!f)
		{
			strcat(configFileName, " is not found.\n");
			throw FilePanic(0,configFileName);
		}
		
		while((string[counter++] = fgetc(f)) != EOF);

		counter--;
		string[counter] = NULL;
		fclose(f);

		//if (counter > 30)
		//{
		//	throw FilePanic(0,"Stopped program because of more than 30 ports described in tcptunnel.cfg\n"); 
		//}
	}
	catch (FilePanic filePanic) 
	{
		printf("%s\n",filePanic.reason());
		exit(0);
	}

	char seps[]   = " ,\t\n";
	char *token;

	unsigned threadID;
	struct ConnectInfo tcpCi[30];
	struct ConnectInfo udpCi[30];
	
	token = strtok( string, seps );

	int tcpCounter = 0;
	int udpCounter = 0;
	
	try
	{
		bool flag = true;

		while( token != NULL )
		{
			if ((strcmp(token,"tcp") == 0) || (strcmp(token,"TCP") == 0))
			{
				printf("%s\t",token);
				if ((token = strtok( NULL, seps )) == NULL)
					flag = false;

				printf("%s\t",token);
				tcpCi[tcpCounter].clientPort = atoi(token);

				if ((token = strtok( NULL, seps )) == NULL)
					flag = false;

				printf("%s\t",token);
				strcpy(tcpCi[tcpCounter].backEndServerIPAddr, token);

				if ((token = strtok( NULL, seps )) == NULL)
					flag = false;

				printf("%s\n",token);
				tcpCi[tcpCounter].serverPort = atoi(token);	
				
				token = strtok( NULL, seps );

				if (flag == false)
					throw FilePanic(0,"File Parse Error");

				tcpCounter++;
			}
			else if ((strcmp(token,"udp") == 0) || (strcmp(token,"UDP") == 0))
			{
				if ((token = strtok( NULL, seps )) == NULL)
					flag = false;

				printf("%s\t",token);
				udpCi[udpCounter].clientPort = atoi(token);

				if ((token = strtok( NULL, seps )) == NULL)
					flag = false;
				
				printf("%s\t",token);
				strcpy(udpCi[udpCounter].backEndServerIPAddr, token);

				if ((token = strtok( NULL, seps )) == NULL)
					flag = false;

				printf("%s\n",token);
				udpCi[udpCounter].serverPort = atoi(token);	
				
				token = strtok( NULL, seps );

				if (flag == false)
					throw FilePanic(0,"File Parse Error");

				udpCounter++;
			}
			else
				throw FilePanic(0,"File Parse Error");
		}

	if (( tcpCounter >= 30 ) || ( udpCounter >= 30 ) )
		throw FilePanic(0,"Unable to make more than 30 tcp/udp connection");
	}
	
	catch (FilePanic filePanic) 
	{
		printf("%s\n",filePanic.reason());
		exit(0);
	}

	HANDLE threadHandle[100];

	for(int i = 0; i < tcpCounter; i++)
	{
		threadHandle[i]
			= (HANDLE)_beginthreadex(
			NULL, 0, &_tcpMainThread, &tcpCi[i], 0, &threadID); 
	}

	for(i = 0; i < udpCounter; i++)
	{
		threadHandle[i + tcpCounter]
			= (HANDLE)_beginthreadex(
			NULL, 0, &_udpMainThread, &udpCi[i], 0, &threadID); 
	}
	
		
	WaitForMultipleObjects(
		(tcpCounter + udpCounter), threadHandle, TRUE,INFINITE);

	for(i = 0; i < (tcpCounter + udpCounter); i++)
		CloseHandle(threadHandle[i]);

	return 0;
}


