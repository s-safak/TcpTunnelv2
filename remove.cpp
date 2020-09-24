#include "stdafx.h" 
#include <windows.h>
#include <iostream.h>

void remove(void)
{
	SC_HANDLE service, scm;
	BOOL success;
	SERVICE_STATUS status;

	cout << "Starting...\n";

	scm = OpenSCManager(0, 0, SC_MANAGER_CREATE_SERVICE);
	if (!scm)
	{
		cout << "In CreateService" << endl;
		cout << "Error number: " << GetLastError() << endl;

		ExitProcess(GetLastError());
	}

	service = OpenService(scm, "TCPTunnel",	SERVICE_ALL_ACCESS | DELETE);
	if (!service)
	{
		cout << "In OpenService" << endl;
		cout << "Error number: " << GetLastError() << endl;

		ExitProcess(GetLastError());
	}
	
	success = QueryServiceStatus(service, &status);
	if (!success)
	{
		cout << "In QueryServiceStatus" << endl;
		cout << "Error number: " << GetLastError() << endl;

		ExitProcess(GetLastError());
	}


	if (status.dwCurrentState != SERVICE_STOPPED)
	{
		cout << "Stopping service...\n";
		success = ControlService(service,
			SERVICE_CONTROL_STOP, 
			&status);
		if (!success)
		{
			cout << "In ControlService" << endl;
			cout << "Error number: " << GetLastError() << endl;

			ExitProcess(GetLastError());
		}
		Sleep(500);
	}

	success = DeleteService(service);
	if (success)
		cout << "Service removed\n";
	else
	{
		cout << "In DeleteService" << endl;
		cout << "Error number: " << GetLastError() << endl;

		ExitProcess(GetLastError());
	}

	CloseServiceHandle(service);
	CloseServiceHandle(scm);
	cout << "Ending...\n";
}
