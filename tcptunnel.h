#include<windows.h>
#include<iostream.h>
#include<winsock.h>

#define				NO_FLAG_SET		0
#define				MAXBUFLEN		1024
#define				LOG_FILE		"c:\\temp\\tcptunnel.log"

DWORD	ServiceThread(LPDWORD param);
void	install(void);
void	remove(void);


struct ConnectInfo
{
	int				clientPort;
	char			backEndServerIPAddr[100];
	int				serverPort;
};


class SocketPanic
{
public:
	SocketPanic(int level, char *recvmsg)
	{
		GetLocalTime(&st);

		_itoa(st.wYear,buf,10);
		strcpy(msg,buf);
		strcat(msg,"/");
		_itoa(st.wMonth,buf,10);
		strcat(msg,buf);
		strcat(msg,"/");
		_itoa(st.wDay,buf,10);
		strcat(msg,buf);
		strcat(msg," ");
		_itoa(st.wHour,buf,10);
		strcat(msg,buf);
		strcat(msg,":");
		_itoa(st.wMinute,buf,10);
		strcat(msg,buf);
		strcat(msg,":");
		_itoa(st.wSecond,buf,10);
		strcat(msg,buf);
		strcat(msg," ");

		strcat(msg,recvmsg);
	}

	int severity() 
	{
		return level;
	}
	
	char* reason() 
	{

		FILE *f;

		if ((f = fopen(LOG_FILE, "a")) != NULL)
		{
			fprintf(f,msg);
			fclose(f);
		}

		return msg; 
	}

private:
	char	msg[100];
	int		level;
	SYSTEMTIME	st; 
	char	buf[5];
};

class ThreadPanic
{
public:
	ThreadPanic(int level, char *recvmsg)
	{
		GetLocalTime(&st);

		_itoa(st.wYear,buf,10);
		strcpy(msg,buf);
		strcat(msg,"/");
		_itoa(st.wMonth,buf,10);
		strcat(msg,buf);
		strcat(msg,"/");
		_itoa(st.wDay,buf,10);
		strcat(msg,buf);
		strcat(msg," ");
		_itoa(st.wHour,buf,10);
		strcat(msg,buf);
		strcat(msg,":");
		_itoa(st.wMinute,buf,10);
		strcat(msg,buf);
		strcat(msg,":");
		_itoa(st.wSecond,buf,10);
		strcat(msg,buf);
		strcat(msg," ");

		strcat(msg,recvmsg);
	}

	int severity() 
	{
		return level;
	}
	
	char* reason() 
	{

		FILE *f;

		if ((f = fopen(LOG_FILE, "a")) != NULL)
		{
			fprintf(f,msg);
			fclose(f);
		}

		return msg; 
	}

private:
	char	msg[100];
	int		level;
	SYSTEMTIME	st; 
	char	buf[5];
};

class FilePanic
{
public:
	FilePanic(int level, char *recvmsg)
	{
		GetLocalTime(&st);

		_itoa(st.wYear,buf,10);
		strcpy(msg,buf);
		strcat(msg,"/");
		_itoa(st.wMonth,buf,10);
		strcat(msg,buf);
		strcat(msg,"/");
		_itoa(st.wDay,buf,10);
		strcat(msg,buf);
		strcat(msg," ");
		_itoa(st.wHour,buf,10);
		strcat(msg,buf);
		strcat(msg,":");
		_itoa(st.wMinute,buf,10);
		strcat(msg,buf);
		strcat(msg,":");
		_itoa(st.wSecond,buf,10);
		strcat(msg,buf);
		strcat(msg," ");

		strcat(msg,recvmsg);
	}

	int severity() 
	{
		return level;
	}
	
	char* reason() 
	{

		FILE *f;

		if ((f = fopen(LOG_FILE, "a")) != NULL)
		{
			fprintf(f,msg);
			fclose(f);
		}

		return msg; 
	}

private:
	char	msg[100];
	int		level;
	SYSTEMTIME	st; 
	char	buf[5];	
};

class TCPTunnelServer
{
public:

	TCPTunnelServer(
		int clientPort,
		char* BackEndServerIPAddr,
		int BackEndServerPort);			// Constructor

	connect();
	start();
	close();
	digTunnel();

private:

	SOCKET		listenSocket;
	SOCKET		clientSocket;
	SOCKET		serverSocket;

	int			clientPort;
	char		BackEndServerIPAddr[100];
	int			BackEndServerPort;
};

class UDPTunnelServer
{
public:

	UDPTunnelServer(
		int clientPort,
		char* BackEndServerIPAddr,
		int BackEndServerPort);			// Constructor

	start();
	close();

private:

	int			clientPort;
	char		BackEndServerIPAddr[100];
	int			BackEndServerPort;
};


class TCPTunnel
{
public:
	TCPTunnel(SOCKET clientSocket, SOCKET serverSocket);	// Constructor
	void start();
	void returnWay();

private:
	SOCKET		clientSocket;
	SOCKET		serverSocket;
	bool		lifeFlag;

};
