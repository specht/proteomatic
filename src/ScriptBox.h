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
// #include <QtWebKit>
#include "ConsoleTextEdit.h"
#include "FoldedHeader.h"
#include "IDesktopBox.h"
#include "IFileBox.h"
#include "IScriptBox.h"
#include "DesktopBox.h"
#include "HintLineEdit.h"
#include "NoSlashValidator.h"
// #include "ZoomableWebView.h"


class k_Proteomatic;

class k_ScriptBox: public k_DesktopBox, public IScriptBox
{
    Q_OBJECT
public:
    k_ScriptBox(QSharedPointer<IScript> ak_pScript, k_Desktop* ak_Parent_, k_Proteomatic& ak_Proteomatic);
    virtual ~k_ScriptBox();
    
    virtual QString description();
    virtual IScript* script();
    virtual bool checkReady(QString& as_Error);
    virtual bool checkReadyToGo();
    virtual QStringList iterationKeys();
    virtual QString scriptOutputDirectory() const;
    virtual QString boxOutputDirectory() const;
    virtual void setBoxOutputPrefix(const QString& as_Prefix);
    virtual void setBoxOutputDirectory(const QString& as_Directory);
    virtual QStringList outputFilesForKey(QString as_Key) const;
    virtual QWidget* paneWidget();
    virtual bool hasExistingOutputFiles();
    virtual bool hasExistingOutputFilesForAllIterations();
    virtual bool iterationHasNoExistingOutputFiles(const QString& as_Key);
    virtual bool outputFileActivated(const QString& as_Key);
    virtual void setOutputFileActivated(const QString& as_Key, bool ab_Flag);
    virtual bool useShortIterationTags();
    virtual void setUseShortIterationTags(bool ab_Flag);
    virtual IDesktopBox* boxForOutputFileKey(const QString& as_Key);
    virtual QString boxOutputPrefix() const;
    virtual void update();
    virtual bool isExpanded() const;
    
public slots:
    virtual void start(const QString& as_IterationKey);
    virtual void abort();
    virtual void proposePrefixButtonClicked(bool ab_NotifyOnFailure = true);
    virtual void clearPrefixButtonClicked();
    virtual void clearOutputDirectoryButtonClicked();
    virtual void addOutput(QString as_String);
    virtual void refreshOutputFileView();
    virtual void setExpanded(bool ab_Flag);
    
protected slots:
    virtual void outputFileActionToggled();
    virtual void handleBoxDisconnected(IDesktopBox* ak_Other_, bool ab_Incoming);
    virtual void readyReadSlot();
    virtual void showOutputBox(bool ab_Flag = true);
    virtual void showOutputFileBox(bool ab_Flag = true);
    virtual void scriptParameterChanged(const QString& as_Key);
    virtual void chooseOutputDirectory();
    virtual void hidingBuddy();
    virtual void showingBuddy();
    virtual void outputBoxIterationKeyChooserChanged();
    virtual void toggleUi();
    virtual void toggleOutputFileChooser(int ai_Index);
    virtual void zoomWebView(int ai_Delta);
    virtual void outputFilenameDetailsChanged();
    virtual void linkClickedSlot(const QUrl& ak_Url);
    
signals:
    void scriptStarted();
    void scriptFinished(int ai_ExitCode);
    void readyRead();
    void outputDirectoryChanged();
    
protected:
    virtual void setupLayout();
    virtual void dragEnterEvent(QDragEnterEvent* event);
    virtual void dragMoveEvent(QDragMoveEvent* event);
    virtual void dropEvent(QDropEvent* event);
    virtual Qt::DropActions supportedDropActions() const;
    QString finishedOutputFilename(QString& as_Filename);
    QStringList inputFilenamesForBox(IFileBox* ak_Box_, QString as_Tag);
    
    QSharedPointer<IScript> mk_pScript;
    QSharedPointer<QWidget> mk_pParameterProxyWidget;
    QHash<QString, IDesktopBox*> mk_OutputFileBoxes;
    QHash<QString, QCheckBox*> mk_Checkboxes;
    QString ms_CurrentIterationKeyRunning;
    QString ms_CurrentIterationKeyShowing;
    
    k_HintLineEdit mk_Prefix;
    k_HintLineEdit mk_OutputDirectory;

    QHash<QString, QSharedPointer<k_ConsoleTextEdit> > mk_Output;
    QWidget* mk_OutputBoxContainer_;
    QWidget* mk_OutputFileViewerContainer_;
    QWidget* mk_OutputBoxIterationKeyChooserContainer_;
    QComboBox* mk_OutputBoxIterationKeyChooser_;
    QTabWidget* mk_TabWidget_;
    QWidget* mk_IterationsTagsDontMatchIcon_;
    QCheckBox* mk_UseShortTagsCheckBox_;
    QString ms_ConverterFilenamePattern;
    QSet<QString> mk_FilenameAffectingParameters;
    QSize mk_LastUserAdjustedSize;
    
    // this is the path to the input file which has been chosen 
    // for determining the automatic output directory 
    QString ms_OutputDirectoryDefiningInputPath;
    QHash<QString, QStringList> mk_InputFilesForKey;
    QSet<QString> mk_SnippetInputFiles;
    QHash<QString, QStringList> mk_OutputFilesForKey;
    QStringList mk_IterationTags;
    QHash<QString, QSet<QString> > mk_OutputFilesForIterationTag;
    bool mb_IterationsTagsDontMatch;
    bool mb_MultipleInputBatches;
    k_NoSlashValidator mk_NoSlashValidator;
    QComboBox* mk_OutputFileChooser_;
//     k_ZoomableWebView* mk_WebView_;
    QStringList mk_PreviewSuffixes;
    k_FoldedHeader* mk_FoldedHeader_;
    QBoxLayout* mk_OutputBoxLayout_;
};
