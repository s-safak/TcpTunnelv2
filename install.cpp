// install.cpp

#include "stdafx.h" 
#include <windows.h>
#include <iostream.h>

void install(void)
{
	SC_HANDLE newService, scm;

	cout << "Starting...\n";	// open a connection to the SCM

	scm = OpenSCManager(0, 0,SC_MANAGER_CREATE_SERVICE);
	if (!scm)
	{
		cout << "In OpenScManager" << endl;
		cout << "Error number: " << GetLastError() << endl;

		ExitProcess(GetLastError());
	}

	// Get current directory

	char buffer[MAX_PATH];

	DWORD len = GetCurrentDirectory(MAX_PATH, buffer);
	
	if (len == 0) 
		cout << "Could not get current Directory...\n";
	
	char s[MAX_PATH];
    strcpy(s, buffer);
	strcat(s, "\\tcptunnel4nts.exe -exec");


	// Install the new service

	newService = CreateService(
		scm, 
		"TCPTunnel",
		"TCPTunnel",
		SERVICE_ALL_ACCESS,
		SERVICE_WIN32_OWN_PROCESS,
		SERVICE_DEMAND_START,
		SERVICE_ERROR_NORMAL,
		s,
		0, 0, 0, 0, 0);
	if (!newService)
	{
		cout << "In CreateService" << endl;
		cout << "Error number: " << GetLastError() << endl;

		ExitProcess(GetLastError());
	}
	else
		cout << "Service installed\n";

	// clean up
	CloseServiceHandle(newService);
	CloseServiceHandle(scm);
	cout << "Ending...\n";
}
