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
#include "ClickableLabel.h"
#include "ConsoleString.h"
#include "FileList.h"
#include "Proteomatic.h"
#include "StopWatch.h"
#include "IScript.h"
#include "UnclickableLabel.h"
#include <math.h>
#include <stdlib.h>


class k_Desktop;
class k_ScriptBox;
class IFileBox;
class k_InputFileBox;
class k_InputFileListBox;
class k_OutputFileBox;


struct r_BoxStatus
{
	enum Enumeration
	{
		Ready = 0,
		OutputFileExists = 1,
		InputFilesMissing = 2,
		Running = 3,
		Finished = 4,
		Failed = 5
	};
};


struct r_InputFileListBoxType
{
	enum Enumeration
	{
		List = 0,
		Batch
	};
};


class k_DesktopBox: public QWidget
{
	Q_OBJECT
public:
	k_DesktopBox(k_Desktop* ak_Parent_, k_Proteomatic& ak_Proteomatic);
	virtual ~k_DesktopBox();
	
	typedef QHash<QString, QString> tk_StringStringHash;
	
	r_BoxStatus::Enumeration status() const;
	
signals:
	void moved();
	void resized();
	void mousePressed(Qt::KeyboardModifiers ae_Modifiers);
	void arrowPressed();
	void arrowReleased();
	void changed();
	
public slots:
	virtual void updateStatus();
	virtual void reportStatus();
	virtual void snapToGrid();
	
protected slots:
	void setSpecialFrame(bool ab_Flag);

protected:
	virtual void paintEvent(QPaintEvent* ak_Event_);
	virtual void mousePressEvent(QMouseEvent* ak_Event_);
	virtual void mouseReleaseEvent(QMouseEvent* ak_Event_);
	virtual void mouseMoveEvent(QMouseEvent* ak_Event_);
	virtual void moveEvent(QMoveEvent* ak_Event_);
	virtual void resizeEvent(QResizeEvent* ak_Event_);
	
	void setKeepSmall(bool ab_Flag);
	bool cursorWithinSizeGrip(QPoint ak_Position);

	k_Desktop* mk_Desktop_;
	QBrush mk_Background;
	QPen mk_Border;
	k_Proteomatic& mk_Proteomatic;
	QLabel* mk_SizeGripLabel_;
	bool mb_Moving;
	bool mb_Resizing;
	QPoint mk_OldMousePosition;
	QPoint mk_OldPosition;
	QHash<k_DesktopBox*, QPoint> mk_OtherBoxesOldPosition;
	QSize mk_OldSize;
	bool mb_KeepSmall;
	bool mb_SpecialFrame;
	int mi_GridSize;
	r_BoxStatus::Enumeration me_Status;
};


class k_ScriptBox: public k_DesktopBox
{
	Q_OBJECT
public:
	k_ScriptBox(RefPtr<IScript> ak_pScript, k_Desktop* ak_Parent_, k_Proteomatic& ak_Proteomatic);
	virtual ~k_ScriptBox();
	
	QList<k_OutputFileBox*> outputFileBoxes();
	RefPtr<IScript> script();
	bool allInputFilesExist();
	
signals:
	void scriptFinished();
	
public slots:
	virtual void updateStatus();
	virtual void reportStatus();
	void toggleOutputFile(QString as_Key, bool ab_Enabled, bool ab_ToggleCheckBox = true);
	void fileBoxConnected(IFileBox* ak_FileBox_);
	void fileBoxDisconnected(IFileBox* ak_FileBox_);
	void removeOutputFileBox(k_OutputFileBox* ak_OutputFileBox_);
	void resetScript();
	void start();

protected slots:
	void toggleOutput(bool ab_Enabled);
	void showParameterWidget();
	void prefixChanged(const QString& as_Prefix);
	void proposePrefixButtonClicked();
	void fileBoxChanged();
	void showProfileManager();
	
	void scriptStarted();
	void scriptFinished(int, QProcess::ExitStatus);
	void scriptReadyRead();
	void addOutput(QString as_Text);

protected:
	RefPtr<IScript> mk_pScript;
	RefPtr<QWidget> mk_pParameterWidget;
	QHash<QString, k_OutputFileBox*> mk_OutputFileBoxes;
	// remember the checkbox that activated an output file box
	QHash<k_OutputFileBox*, QCheckBox*> mk_CheckBoxForOutputFileBox;
	
	// for each file box, keep a hash of filename -> input group key
	QHash<IFileBox*, tk_StringStringHash> mk_InputFileBoxes;
	
	QVBoxLayout mk_Layout;
	QToolButton mk_StatusLabel;
	QLineEdit mk_PrefixWidget;
	QString ms_InputFilesErrorMessage;
	k_ConsoleString ms_Output;
	RefPtr<QWidget> mk_pOutputWidget;
	RefPtr<QTextEdit> mk_pOutput;
};


struct IFileBox
{
	IFileBox() {};
	virtual ~IFileBox() {};
	
	virtual QStringList fileNames() = 0;
};


// converter scripts convert each input file of a specific type into an output file
// therefore, output files are not optional, but are defined by the list of input files
// converter output files appear in the converter script box itself, so this is a 
// script box with file box properties (arrow and file list)
class k_ConverterScriptBox: public k_ScriptBox, public IFileBox
{
public:
	k_ConverterScriptBox(RefPtr<IScript> ak_pScript, k_Desktop* ak_Parent_, k_Proteomatic& ak_Proteomatic);
	virtual ~k_ConverterScriptBox();
	
	virtual QStringList fileNames();
	
protected:
	QListWidget mk_FileList;
};


class k_InputFileBox: public k_DesktopBox, public IFileBox
{
	Q_OBJECT
public:
	k_InputFileBox(k_Desktop* ak_Parent_, k_Proteomatic& ak_Proteomatic);
	virtual ~k_InputFileBox();

	virtual QStringList fileNames();
	void setFilename(const QString& as_Filename);
	QString filename() const;
	
public slots:
	virtual void updateStatus();
	virtual void reportStatus();

protected:
	virtual bool fileExists();
	virtual QString displayString() const;
	
	QToolButton mk_IconLabel;
	k_UnclickableLabel mk_Label;
	QString ms_Filename;
};


class k_InputFileListBox: public k_DesktopBox, public IFileBox
{
	Q_OBJECT
public:
	k_InputFileListBox(k_Desktop* ak_Parent_, k_Proteomatic& ak_Proteomatic);
	virtual ~k_InputFileListBox();
	
	virtual QStringList fileNames();
	void addFilename(const QString& as_Filename);
	
	bool isFileBatch();
	
protected slots:
	void addFilesButtonClicked();
	void updateStatus();
	void toggleUi();
	
protected:
	virtual void dragEnterEvent(QDragEnterEvent* ak_Event_);
	virtual void dragMoveEvent(QDragMoveEvent* ak_Event_);
	virtual void dropEvent(QDropEvent* ak_Event_);
	
	k_FileList mk_FileList;
	k_UnclickableLabel mk_Label;
	QToolButton mk_RemoveFilesButton;
	QToolButton mk_BatchModeButton;
	r_InputFileListBoxType::Enumeration me_Type;
};


class k_OutputFileBox: public k_InputFileBox
{
	Q_OBJECT
public:
	k_OutputFileBox(k_Desktop* ak_Parent_, k_Proteomatic& ak_Proteomatic, k_ScriptBox& ak_ScriptBox);
	virtual ~k_OutputFileBox();
	
	virtual QStringList fileNames();
	QString directory();
	QString prefix();
	QString assembledPath();
	
public slots:
	void setDirectory(const QString& as_Directory);
	void setPrefix(const QString& as_Prefix);
	virtual void updateStatus();
	virtual void reportStatus();
	
protected:
	virtual bool fileExists();
	virtual QString displayString() const;
	
	QString ms_Directory;
	QString ms_Prefix;
	k_ScriptBox& mk_ScriptBox;
};
