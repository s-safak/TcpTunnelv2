#include	"stdafx.h"
#include	<windows.h>
#include	<stdio.h>
#include	<iostream.h>
#include	<stdlib.h>
#include	"tcptunnel.h"
#include	<winsock.h>

char *SERVICE_NAME = "tcptunnelService";

HANDLE terminateEvent = NULL;
SERVICE_STATUS_HANDLE serviceStatusHandle;

BOOL pauseService = FALSE;
BOOL runningService = FALSE;

PROCESS_INFORMATION pi;

HANDLE threadHandle = 0;

void ErrorHandler(char *s, DWORD err)
{
	cout << s << endl;
	cout << "Error number: " << err << endl;
	ExitProcess(err);
}

BOOL InitService()
{
	DWORD id;

	threadHandle = CreateThread(0, 0,
		(LPTHREAD_START_ROUTINE) ServiceThread,
		0, 0, &id);

	if (threadHandle==0)
		return FALSE;
	else
	{
		runningService = TRUE;
		return TRUE;
	}
}


VOID ResumeService()
{
	pauseService=FALSE;
	ResumeThread(threadHandle);
}

VOID PauseService()
{
	pauseService = TRUE;
	SuspendThread(threadHandle);
}


VOID StopService() 
{
	WSACleanup();
	
	runningService=FALSE;

	TerminateProcess(pi.hProcess, 0);

	SetEvent(terminateEvent);
}

BOOL SendStatusToSCM (DWORD dwCurrentState,
	DWORD dwWin32ExitCode, 
	DWORD dwServiceSpecificExitCode,
	DWORD dwCheckPoint,
	DWORD dwWaitHint)
{
	BOOL success;
	SERVICE_STATUS serviceStatus;

	serviceStatus.dwServiceType =
		SERVICE_WIN32_OWN_PROCESS;
	serviceStatus.dwCurrentState = dwCurrentState;

	if (dwCurrentState == SERVICE_START_PENDING)
		serviceStatus.dwControlsAccepted = 0;
	else
		serviceStatus.dwControlsAccepted = 
			SERVICE_ACCEPT_STOP |
			SERVICE_ACCEPT_PAUSE_CONTINUE |
			SERVICE_ACCEPT_SHUTDOWN;

	if (dwServiceSpecificExitCode == 0)
		serviceStatus.dwWin32ExitCode =
			dwWin32ExitCode;
	else
		serviceStatus.dwWin32ExitCode = 
			ERROR_SERVICE_SPECIFIC_ERROR;
	serviceStatus.dwServiceSpecificExitCode =
		dwServiceSpecificExitCode;

	serviceStatus.dwCheckPoint = dwCheckPoint;
	serviceStatus.dwWaitHint = dwWaitHint;

	success = SetServiceStatus (serviceStatusHandle,
		&serviceStatus);
	if (!success)
		StopService();

	return success;
}


VOID ServiceCtrlHandler (DWORD controlCode) 
{
	DWORD  currentState = 0;
	BOOL success;

	switch(controlCode)
	{
		case SERVICE_CONTROL_STOP:
			currentState = SERVICE_STOP_PENDING;

			success = SendStatusToSCM(
				SERVICE_STOP_PENDING,
				NO_ERROR, 0, 1, 5000);

			StopService();
			return;

		case SERVICE_CONTROL_PAUSE:
			if (runningService && !pauseService)
			{
				success = SendStatusToSCM(
					SERVICE_PAUSE_PENDING,
					NO_ERROR, 0, 1, 1000);
				PauseService();
				currentState = SERVICE_PAUSED;
			}
			break;

		case SERVICE_CONTROL_CONTINUE:
			if (runningService && pauseService)
			{
				success = SendStatusToSCM(
					SERVICE_CONTINUE_PENDING,
					NO_ERROR, 0, 1, 1000);
					ResumeService();
					currentState = SERVICE_RUNNING;
			}
			break;

		case SERVICE_CONTROL_INTERROGATE:
			break;

		case SERVICE_CONTROL_SHUTDOWN:
			return;
		default:
 			break;
	}
	SendStatusToSCM(currentState, NO_ERROR,
		0, 0, 0);
}


VOID terminate(DWORD error)
{
	if (terminateEvent)
		CloseHandle(terminateEvent);

	if (serviceStatusHandle)
		SendStatusToSCM(SERVICE_STOPPED, error,
			0, 0, 0);

	if (threadHandle)
		CloseHandle(threadHandle);
}


VOID ServiceMain(DWORD argc, LPTSTR *argv) 
{
	BOOL success;

	serviceStatusHandle =
		RegisterServiceCtrlHandler(
			SERVICE_NAME,
			(LPHANDLER_FUNCTION) ServiceCtrlHandler);
	if (!serviceStatusHandle)
	{
		terminate(GetLastError());
		return;
	}

	success = SendStatusToSCM(
		SERVICE_START_PENDING,
		NO_ERROR, 0, 1, 5000);
	if (!success)
	{
		terminate(GetLastError()); 
		return;
	}

	terminateEvent = CreateEvent (0, TRUE, FALSE,0);
	if (!terminateEvent)
	{
		terminate(GetLastError());
		return;
	}

	success = SendStatusToSCM(
		SERVICE_START_PENDING,NO_ERROR, 0, 2, 1000);
	if (!success)
	{
		terminate(GetLastError()); 
		return;
	}

	success = SendStatusToSCM(
		SERVICE_START_PENDING,
		NO_ERROR, 0, 3, 5000);
	if (!success)
	{
		terminate(GetLastError()); 
		return;
	}

	success = InitService();
	if (!success)
	{
		terminate(GetLastError());
		return;
	}

	success = SendStatusToSCM(
		SERVICE_RUNNING,
		NO_ERROR, 0, 0, 0);
	if (!success)
	{
		terminate(GetLastError()); 
		return;
	}

	WaitForSingleObject (terminateEvent, INFINITE);

	terminate(0);
}



VOID main(int argc, char* argv[])
{
	if (!((argc == 2) && (strcmp("-exec", argv[1]) == 0)))
	{
		if (argc == 2) 
		{
			if (strcmp("-install", argv[1]) == 0)
				install();
			else if (strcmp("-remove", argv[1]) == 0)
				remove();

			exit(0);
		}
		else 
		{
			printf("Usage: \n");
			printf("tcptunnel4nts -install\n");
			printf("	This is to install tcptunnel4nts as a NT Service\n");
			printf("tcptunnel4nts -remove\n");
			printf("	This is to remove tcptunnel4nts\n");
			printf("After install it, you can start tcptunnel4nts from NT Services Panel.\n\n");
		
			exit(0);
		}
	}


	SERVICE_TABLE_ENTRY serviceTable[] = 
	{ 
	{ SERVICE_NAME,
		(LPSERVICE_MAIN_FUNCTION) ServiceMain},
	{ NULL, NULL }
	};
	BOOL success;

	success = 
		StartServiceCtrlDispatcher(serviceTable);
	if (!success)
		ErrorHandler("In StartServiceCtrlDispatcher",
			GetLastError());
}

