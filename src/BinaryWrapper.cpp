#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef WIN32
#include <windows.h>
int WinMain(HINSTANCE,HINSTANCE,LPSTR commandLine,int)
#else
int main(int argc, char** argv__)
#endif
{
    char* DIR = new char[16384];
    char* SEARCH_PATTERN = new char[16384];
    strcpy(SEARCH_PATTERN, BINARY);
#ifdef WIN32
    strcpy(DIR, GetCommandLine());
    
    *(strstr(DIR, SEARCH_PATTERN)) = 0;
    strcat(DIR, "bin\\");
#else
    strcpy(DIR, argv__[0]);
    DIR[strlen(DIR) - strlen(BINARY)] = 0;
    strcat(DIR, "bin/");
#endif
#ifdef WIN32
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );
#endif
    char* ls_Dir_ = new char[16384];
    strcpy(ls_Dir_, DIR);
    strcat(ls_Dir_, BINARYCORE);
    char* ls_DirUpdated_ = new char[16384];
    strcpy(ls_DirUpdated_, ls_Dir_);
    strcat(ls_DirUpdated_, "_updated");
    
#ifdef WIN32
    DWORD li_ReturnCode = 0;
#else
    int li_ReturnCode = 0;
#endif
    do
    {
#ifdef WIN32
        CreateProcess(ls_Dir_, NULL, NULL, NULL, FALSE, 0, NULL, DIR, &si, &pi);
        WaitForSingleObject(pi.hProcess,INFINITE);
        GetExitCodeProcess(pi.hProcess, &li_ReturnCode);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
#else
        li_ReturnCode = system(ls_Dir_);
#endif
        // rename ProteomaticCore_updated to ProteomaticCore
        FILE* f = fopen(ls_DirUpdated_, "r");
        if (f)
        {
            fclose(f);
            rename(ls_DirUpdated_, ls_Dir_);
        }
    } while ((li_ReturnCode & 0xff00) == 0x1400);
    delete [] ls_Dir_;
    return li_ReturnCode;
}

