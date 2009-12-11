#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>

#ifdef WIN32
int WinMain(HINSTANCE,HINSTANCE,LPSTR,int)
#else
int main(int argc, char** argv__)
#endif
{
	char* PATH = new char[5];
	strcpy(PATH, "bin/");
#ifdef WIN32
	PATH[3] = '\\';
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );
#endif
	char* ls_Path_ = BINARY;
	
	char* ls_Dir_ = new char[strlen(ls_Path_) + 1 + strlen(PATH) + strlen(BINARY) + 4];
	strcpy(ls_Dir_, PATH);
	strcat(ls_Dir_, ls_Path_);
	
#ifdef WIN32
	DWORD li_ReturnCode = 0;
#else
	int li_ReturnCode = 0;
#endif
	do
	{
#ifdef WIN32
    CreateProcess(ls_Dir_, NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
    WaitForSingleObject(pi.hProcess,INFINITE);
    GetExitCodeProcess(pi.hProcess, &li_ReturnCode);
    CloseHandle( pi.hProcess );
    CloseHandle( pi.hThread );
#else
		li_ReturnCode = system(ls_Dir_);
#endif
	} while ((li_ReturnCode & 0xff00) == 0x1400);
	delete [] ls_Dir_;
    return li_ReturnCode;
}

