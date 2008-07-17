#include <QtCore>
#include <QtGui>
#include "Proteomatic.h"
#include "PipelineMainWindow.h"


int main(int ai_ArgumentCount, char** ac_Arguments__)
{
    Q_INIT_RESOURCE(Proteomatic);
	QApplication lk_App(ai_ArgumentCount, ac_Arguments__);
	
	k_Proteomatic lk_Proteomatic(lk_App.applicationDirPath());
	
	k_PipelineMainWindow lk_MainWindow(NULL, lk_Proteomatic);
	return lk_App.exec();
}
