#include <stdlib.h>
#include <stdio.h>
#include <string.h>


// BINARY
#define PATH "bin/"


int main(int argc, char** argv__)
{
	char* ls_Path_ = argv__[0];
	ls_Path_[strlen(ls_Path_) - strlen(BINARY)] = 0;
	
	char* ls_Dir_ = new char[strlen(ls_Path_) + 1 + strlen(PATH) + strlen(BINARY)];
	strcpy(ls_Dir_, ls_Path_);
	strcat(ls_Dir_, PATH);
	strcat(ls_Dir_, BINARY);
	
	int li_ReturnCode = 0;
	do
	{
		li_ReturnCode = system(ls_Dir_);
	} while (li_ReturnCode == 0x1300);
	delete [] ls_Dir_;
}
