/*
Copyright (c) 2007-2008 Michael Specht

This file is part of Proteomatic.

Proteomatic is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Proteomatic is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Proteomatic.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <QtGui>
#include "FileList.h"
#include "Proteomatic.h"
#include "StopWatch.h"
#include "Script.h"
#include "UnclickableLabel.h"
#include <math.h>
#include <stdlib.h>


class k_Desktop;
class k_ScriptBox;
class k_FileBox;
class k_InputFileBox;
class k_InputFileListBox;
class k_OutputFileBox;


class k_DesktopBox: public QWidget
{
	Q_OBJECT
public:
	k_DesktopBox(k_Desktop* ak_Parent_, k_Proteomatic& ak_Proteomatic);
	virtual ~k_DesktopBox();
	
	typedef QHash<QString, QString> tk_StringStringHash;
	
signals:
	void moved();
	void resized();
	void mousePressed(Qt::KeyboardModifiers ae_Modifiers);

protected:
	virtual void paintEvent(QPaintEvent* ak_Event_);
	virtual void mousePressEvent(QMouseEvent* ak_Event_);
	virtual void mouseReleaseEvent(QMouseEvent* ak_Event_);
	virtual void mouseMoveEvent(QMouseEvent* ak_Event_);
	virtual void moveEvent(QMoveEvent* ak_Event_);
	virtual void resizeEvent(QResizeEvent* ak_Event_);
	
	void setKeepSmall(bool ab_Flag);
	bool cursorWithinSizeGrip(QPoint ak_Position);

protected:
	k_Desktop* mk_Desktop_;
	QBrush mk_Background;
	QPen mk_Border;
	k_Proteomatic& mk_Proteomatic;
	QLabel* mk_SizeGripLabel_;
	bool mb_Moving;
	bool mb_Resizing;
	QPoint mk_OldMousePosition;
	QPoint mk_OldPosition;
	QSize mk_OldSize;
	bool mb_KeepSmall;
};


class k_ScriptBox: public k_DesktopBox
{
	Q_OBJECT
public:
	k_ScriptBox(QString as_ScriptName, k_Desktop* ak_Parent_, k_Proteomatic& ak_Proteomatic);
	virtual ~k_ScriptBox();
	
	QList<k_OutputFileBox*> outputFileBoxes();
	
public slots:
	void updateStatus();
	void fileBoxConnected(k_FileBox* ak_FileBox_);
	void fileBoxDisconnected(k_FileBox* ak_FileBox_);
	void removeOutputFileBox(k_OutputFileBox* ak_OutputFileBox_);

protected slots:
	void toggleOutput(bool ab_Enabled);
	void showParameterWidget();
	void prefixChanged(const QString& as_Prefix);
	void proposePrefixButtonClicked();
	void fileBoxChanged();
	void showProfileManager();

protected:
	k_Script* mk_Script_;
	RefPtr<QWidget> mk_pParameterWidget;
	QHash<QString, k_OutputFileBox*> mk_OutputFileBoxes;
	// remember the checkbox that activated an output file box
	QHash<k_OutputFileBox*, QCheckBox*> mk_CheckBoxForOutputFileBox;
	
	// for each file box, keep a hash of filename -> input group key
	QHash<k_FileBox*, tk_StringStringHash> mk_InputFileBoxes;
	
	QVBoxLayout mk_Layout;
	QLabel mk_StatusLabel;
	QLineEdit mk_PrefixWidget;
};


class k_FileBox: public k_DesktopBox
{
	Q_OBJECT
public:
	k_FileBox(k_Desktop* ak_Parent_, k_Proteomatic& ak_Proteomatic);
	virtual ~k_FileBox();
	
	virtual QStringList fileNames() = 0;
	
signals:
	void arrowPressed();
	void arrowReleased();
	void changed();
};


class k_InputFileBox: public k_FileBox
{
	Q_OBJECT
public:
	k_InputFileBox(k_Desktop* ak_Parent_, k_Proteomatic& ak_Proteomatic);
	virtual ~k_InputFileBox();

	virtual QStringList fileNames();
	void setFilename(const QString& as_Filename);
	QString filename() const;
	
public slots:
	void updateStatus();

protected:
	virtual bool fileExists();
	virtual QString displayString() const;
	
	QLabel mk_Label;
	QString ms_Filename;
};


class k_InputFileListBox: public k_FileBox
{
	Q_OBJECT
public:
	k_InputFileListBox(k_Desktop* ak_Parent_, k_Proteomatic& ak_Proteomatic);
	virtual ~k_InputFileListBox();
	
	virtual QStringList fileNames();
	void addFilename(const QString& as_Filename);
	
protected slots:
	void addFilesButtonClicked();
	void updateStatus();
	
protected:
	virtual void dragEnterEvent(QDragEnterEvent* ak_Event_);
	virtual void dragMoveEvent(QDragMoveEvent* ak_Event_);
	virtual void dropEvent(QDropEvent* ak_Event_);
	
	k_FileList mk_FileList;
	k_UnclickableLabel mk_Label;
};


class k_OutputFileBox: public k_InputFileBox
{
	Q_OBJECT
public:
	k_OutputFileBox(k_Desktop* ak_Parent_, k_Proteomatic& ak_Proteomatic);
	virtual ~k_OutputFileBox();
	
	virtual QStringList fileNames();
	QString directory();
	QString prefix();
	QString assembledPath();
	
public slots:
	void setDirectory(const QString& as_Directory);
	void setPrefix(const QString& as_Prefix);
	
protected:
	virtual bool fileExists();
	virtual QString displayString() const;
	
	QString ms_Directory;
	QString ms_Prefix;
};
