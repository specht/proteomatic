#include <QtGui>
#include "ScriptHelper.h"
#include "Proteomatic.h"


int main(int ai_ArgumentCount, char** ac_Arguments__)
{
    Q_INIT_RESOURCE(Proteomatic);
	QApplication lk_App(ai_ArgumentCount, ac_Arguments__);
	
	k_Proteomatic lk_Proteomatic(lk_App.applicationDirPath());
	
	k_ScriptHelper lk_ScriptHelper(NULL, lk_Proteomatic);
	lk_ScriptHelper.show();
	return lk_App.exec();
}
