#include <QtCore>
#include <QtGui>


class k_SizeWatchWidget: public QWidget
{
	Q_OBJECT
	
public:
	k_SizeWatchWidget(QWidget* ak_Parent_ = 0, Qt::WindowFlags ae_Flags = 0) 
		: QWidget(ak_Parent_, ae_Flags)
	{ 
	}
	
	virtual ~k_SizeWatchWidget()
	{
	}
	
protected:
	virtual void resizeEvent(QResizeEvent* ak_Event_)
	{
		emit widgetResized();
		ak_Event_->ignore();
	}
	
signals:
	void widgetResized();
};
