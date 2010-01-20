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
    
    char* path_ = new char[16384];
    char* fixedPath_ = new char[16384];

    
#ifdef WIN32
    DWORD li_ReturnCode = 0;
#else
    int li_ReturnCode = 0;
#endif
    bool lb_Repeat = false;
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
        lb_Repeat = ((li_ReturnCode & 0xff00) == 0x1400);
        if (lb_Repeat)
        {
            // rename ProteomaticCore_updated to ProteomaticCore
            FILE* fu = fopen("update-finish.txt", "r");
            if (fu)
            {
                while (!feof(fu))
                {
                    fgets(path_, 16383, fu);
                    // remove trailing newlines
                    while ((strlen(path_) > 0) && (path_[strlen(path_) - 1] == '\n'))
                        path_[strlen(path_) - 1] = 0;
                    if (strlen(path_) > 0)
                    {
                        // fix slashes on Windows
#ifdef WIN32
                        for (int i = 0; i < strlen(path_); ++i)
                            if (path_[i] == '/')
                                path_[i] = '\\';
#endif                        
                        strcpy(fixedPath_, path_);
                        fixedPath_[strlen(fixedPath_) - 8] = 0;
                        unlink(fixedPath_);
                        rename(path_, fixedPath_);
                        /*
                        FILE *fd = fopen("out.txt", "a");
                        fprintf(fd, "[%s]\n[%s]\n", path_, fixedPath_);
                        fclose(fd);
                        */
                    }
                }
                fclose(fu);
                unlink("update-finish.txt");
            }
        }
    } while (lb_Repeat);
    delete [] ls_Dir_;
    return li_ReturnCode;
}

