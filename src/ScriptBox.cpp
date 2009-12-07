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

#include "ScriptBox.h"
#include "FoldedHeader.h"
#include "Desktop.h"
#include "DesktopBoxFactory.h"
#include "FileListBox.h"
#include "HintLineEdit.h"
#include "OutFileListBox.h"
#include "PipelineMainWindow.h"
#include "ScriptFactory.h"
#include "Tango.h"
#include "UnclickableLabel.h"
#include "LocalScript.h"
#include "InputGroupProxyBox.h"


k_ScriptBox::k_ScriptBox(RefPtr<IScript> ak_pScript, k_Desktop* ak_Parent_, k_Proteomatic& ak_Proteomatic)
	: k_DesktopBox(ak_Parent_, ak_Proteomatic, false, false)
	, mk_pScript(ak_pScript)
	, mk_OutputBoxContainer_(NULL)
	, mk_OutputBoxIterationKeyChooserContainer_(NULL)
	, mk_OutputBoxIterationKeyChooser_(NULL)
	, mk_OutputBox_(NULL)
    , mk_IterationsTagsDontMatchIcon_(NULL)
	, mk_LastUserAdjustedSize(0, 0)
	, ms_CurrentIterationKeyRunning(QString())
	, ms_CurrentIterationKeyShowing(QString())
    , mb_IterationsTagsDontMatch(false)
    , mb_MultipleInputBatches(false)
    , mk_UseShortTagsCheckBox_(NULL)
{
	connect(this, SIGNAL(boxDisconnected(IDesktopBox*, bool)), this, SLOT(handleBoxDisconnected(IDesktopBox*, bool)));
	connect(dynamic_cast<QObject*>(mk_pScript.get_Pointer()), SIGNAL(scriptStarted()), this, SIGNAL(scriptStarted()));
	connect(dynamic_cast<QObject*>(mk_pScript.get_Pointer()), SIGNAL(scriptFinished(int)), this, SIGNAL(scriptFinished(int)));
	connect(dynamic_cast<QObject*>(mk_pScript.get_Pointer()), SIGNAL(readyRead()), this, SIGNAL(readyRead()));
	connect(this, SIGNAL(readyRead()), this, SLOT(readyReadSlot()));
	setupLayout();
}


k_ScriptBox::~k_ScriptBox()
{
	// remove mk_pScript's parameter widget from parameter widget proxy
	mk_pScript->parameterWidget()->setParent(NULL);
	foreach (QString ls_Key, mk_OutputFileBoxes.keys())
		mk_Desktop_->removeBox(mk_OutputFileBoxes[ls_Key]);
}


IScript* k_ScriptBox::script()
{
	return mk_pScript.get_Pointer();
}


bool k_ScriptBox::checkReady(QString& as_Error)
{
	QStringList lk_IterationKeys = iterationKeys();
	if (lk_IterationKeys.empty())
		return false;
	return true;
}


bool k_ScriptBox::checkReadyToGo()
{
	// check whether all input files are there
    foreach (QString ls_Key, mk_InputFilesForKey.keys())
    {
        foreach (QString ls_Path, mk_InputFilesForKey[ls_Key])
            if (!QFileInfo(ls_Path).exists())
                return false;
    }
	return true;
}


QStringList k_ScriptBox::iterationKeys()
{
	if (!batchMode())
		return QStringList() << "";
    
    if (mb_IterationsTagsDontMatch)
        return QStringList();
    
    return mk_IterationTags;
}


QString k_ScriptBox::scriptOutputDirectory() const
{
	if (!mk_OutputDirectory.text().isEmpty())
	{
		return mk_OutputDirectory.text();
	}
	
	// if no output directory has been set, return the directory of one of the
	// output directory specifying input files
	
	// ...but, if it's empty, return home path!
	if (ms_OutputDirectoryDefiningInputPath.isEmpty())
		return QDir::homePath();

	return QFileInfo(ms_OutputDirectoryDefiningInputPath).dir().path();
}


QString k_ScriptBox::boxOutputDirectory() const
{
    return mk_OutputDirectory.text();
}


void k_ScriptBox::setBoxOutputPrefix(const QString& as_Prefix)
{
    mk_Prefix.setText(as_Prefix);
}


void k_ScriptBox::setBoxOutputDirectory(const QString& as_Directory)
{
    mk_OutputDirectory.setText(as_Directory);
}


QStringList k_ScriptBox::outputFilesForKey(QString as_Key) const
{
	if (mk_OutputFilesForKey.contains(as_Key))
		return mk_OutputFilesForKey[as_Key];
	else
		return QStringList();
}


QWidget* k_ScriptBox::paneWidget()
{
	return mk_pParameterProxyWidget.get_Pointer();
}


bool k_ScriptBox::hasExistingOutputFiles()
{
    foreach (QString ls_Key, mk_OutputFilesForKey.keys())
        foreach (QString ls_Path, mk_OutputFilesForKey[ls_Key])
            if (QFileInfo(ls_Path).exists())
                return true;
    return false;
}


bool k_ScriptBox::hasExistingOutputFilesForAllIterations()
{
    // this functions returns true if there is at least one iteration
    // which has 0 existing output files
    if (batchMode())
    {
        foreach (QString ls_Tag, mk_IterationTags)
            if (iterationHasNoExistingOutputFiles(ls_Tag))
                return false;
        return true;
    }
    else
        return hasExistingOutputFiles();
}


bool k_ScriptBox::iterationHasNoExistingOutputFiles(const QString& as_Key)
{
    if (!mk_OutputFilesForIterationTag.contains(as_Key))
        return true;
    
    foreach (QString ls_Path, mk_OutputFilesForIterationTag[as_Key])
        if (QFileInfo(ls_Path).exists())
            return false;
    return true;
}


bool k_ScriptBox::outputFileActivated(const QString& as_Key)
{
	if (!mk_Checkboxes.contains(as_Key))
		return false;
	return mk_Checkboxes[as_Key]->isChecked();
}


void k_ScriptBox::setOutputFileActivated(const QString& as_Key, bool ab_Flag)
{
	if (!mk_Checkboxes.contains(as_Key))
		return;
	mk_Checkboxes[as_Key]->setChecked(ab_Flag);
}


bool k_ScriptBox::useShortIterationTags()
{
    return (batchMode() && (mk_UseShortTagsCheckBox_->checkState() == Qt::Checked));
}


void k_ScriptBox::setUseShortIterationTags(bool ab_Flag)
{
    if (batchMode())
        mk_UseShortTagsCheckBox_->setChecked(ab_Flag);
}


IDesktopBox* k_ScriptBox::boxForOutputFileKey(const QString& as_Key)
{
	if (!mk_OutputFileBoxes.contains(as_Key))
		return NULL;
	return mk_OutputFileBoxes[as_Key];
}


QString k_ScriptBox::boxOutputPrefix() const
{
	return mk_Prefix.text();
}


void k_ScriptBox::outputFileActionToggled()
{
	QCheckBox* lk_CheckBox_ = dynamic_cast<QCheckBox*>(sender());
	QString ls_Key = lk_CheckBox_->property("key").toString();
	if (lk_CheckBox_->checkState() == Qt::Checked)
	{
		IDesktopBox* lk_Box_ = 
		k_DesktopBoxFactory::makeOutFileListBox(
			mk_Desktop_, mk_Proteomatic, ls_Key,
			mk_pScript->outputFileDetails(ls_Key)["label"], false);
		dynamic_cast<QObject*>(lk_Box_)->setProperty("key", ls_Key);
		mk_OutputFileBoxes[ls_Key] = lk_Box_;
		mk_Desktop_->addBox(lk_Box_, false);
		mk_Desktop_->connectBoxes(this, lk_Box_);
	}
	else
	{
		if (mk_OutputFileBoxes.contains(ls_Key))
		{
			// only delete the box if it's really there
			if (dynamic_cast<IDesktopBox*>(mk_OutputFileBoxes[ls_Key]))
				mk_Desktop_->removeBox(mk_OutputFileBoxes[ls_Key]);
			mk_OutputFileBoxes.remove(ls_Key);
		}
	}
    update();
	mk_Desktop_->setHasUnsavedChanges(true);
}


void k_ScriptBox::handleBoxDisconnected(IDesktopBox* ak_Other_, bool ab_Incoming)
{
	if (!ab_Incoming)
	{
		// check whether it's an output file box and we have to uncheck the checkbox!
		// :TODO: speed this up, it's slow. maybe another hash?
		QString ls_Key = mk_OutputFileBoxes.key(ak_Other_);
		if (!ls_Key.isEmpty())
			mk_Checkboxes[ls_Key]->setChecked(Qt::Unchecked);
	}
}


/*
void k_ScriptBox::updateOutputFilenames()
{
	this->determineOutputDirectoryDefiningInputFile();
	
	if (batchMode())
	{
		foreach (QString ls_Key, mk_OutputFileBoxes.keys())
		{
			k_OutFileListBox* lk_OutBox_ = dynamic_cast<k_OutFileListBox*>(mk_OutputFileBoxes[ls_Key]);
			QSet<IFileBox*> lk_FileBoxes;
			foreach (IDesktopBox* lk_Box_, mk_ConnectedIncomingBoxes)
			{
				IFileBox* lk_SourceBox_ = dynamic_cast<IFileBox*>(lk_Box_);
				if (lk_SourceBox_ && lk_Box_->batchMode())
					lk_FileBoxes << lk_SourceBox_;
				else
				{
					k_InputGroupProxyBox* lk_ProxyBox_ = dynamic_cast<k_InputGroupProxyBox*>(lk_Box_);
					if (lk_ProxyBox_ && lk_ProxyBox_->batchMode())
						foreach (IFileBox* lk_FileBox_, lk_ProxyBox_->fileBoxes())
							lk_FileBoxes << lk_FileBox_;
				}
			}
			QStringList lk_Filenames;
			// combine files from all batch mode input file list boxes
			foreach (IFileBox* lk_FileBox_, lk_FileBoxes)
			{
				foreach (QString ls_Path, lk_FileBox_->filenames())
				{
					QString ls_OutFilename = mk_pScript->outputFileDetails(ls_Key)["filename"];
					QString ls_Suffix = QFileInfo(ls_OutFilename).completeSuffix();
					ls_OutFilename.remove(ls_OutFilename.length() - ls_Suffix.length() - 1, ls_Suffix.length() + 1);
					QString ls_OutPath = mk_Prefix.text() + lk_FileBox_->tagForFilename(ls_Path) + "-" + ls_OutFilename + "." + ls_Suffix;
					ls_OutPath = ls_OutPath;
					lk_Filenames.append(QFileInfo(QDir(outputDirectory()), ls_OutPath).absoluteFilePath());
				}
			}
			lk_OutBox_->setFilenames(lk_Filenames);
		}
	}
	else
	{
		// use original filename
		foreach (QString ls_Key, mk_OutputFileBoxes.keys())
		{
			k_OutFileListBox* lk_OutBox_ = dynamic_cast<k_OutFileListBox*>(mk_OutputFileBoxes[ls_Key]);
			QStringList lk_Filenames;
			if (mk_pScript->type() == r_ScriptType::Processor)
				lk_Filenames.append(outputDirectory() + "/" + mk_Prefix.text() + mk_pScript->outputFileDetails(ls_Key)["filename"]);
			else
			{
				foreach (IDesktopBox* lk_Box_, mk_ConnectedIncomingBoxes)
				{
					IFileBox* lk_FileBox_ = dynamic_cast<IFileBox*>(lk_Box_);
					if (lk_FileBox_)
					{
						foreach (QString ls_Path, lk_FileBox_->filenames())
						{
							QString ls_Basename = QFileInfo(ls_Path).baseName();
							QString ls_Extension = "." + QFileInfo(ls_Path).completeSuffix();
							QString ls_Filename = QFileInfo(ls_Path).completeBaseName();
							QString ls_DestinationFilename = ms_ConverterFilenamePattern;
							QRegExp lk_RegExp("(#\\{[a-zA-Z0-9_]+\\})", Qt::CaseSensitive, QRegExp::RegExp2);
							int li_Position = 0;
							while ((li_Position = lk_RegExp.indexIn(ls_DestinationFilename)) != -1)
							{
								QString ls_Capture = lk_RegExp.cap(1);
								QString ls_Key = ls_Capture;
								ls_Key.replace("#{", "");
								ls_Key.replace("}", "");
								QString ls_Value = "";
								if (ls_Key == "basename")
									ls_Value = ls_Basename;
								else if (ls_Key == "extension")
									ls_Value = ls_Extension;
								else if (ls_Key == "filename")
									ls_Value = ls_Filename;
								else
								{
									// it must be a parameter!
									ls_Value = mk_pScript->parameterValue(ls_Key);
								}
								ls_DestinationFilename.replace(ls_Capture, ls_Value);
							}
							lk_Filenames.append(outputDirectory() + "/" + mk_Prefix.text() + ls_DestinationFilename);
						}
					}
				}
			}
			lk_OutBox_->setFilenames(lk_Filenames);
		}
	}
	mk_OutputBoxIterationKeyChooserContainer_->setVisible(batchMode());
	QStringList lk_IterationKeys = iterationKeys();
	
	QStringList lk_Items;
	for (int i = 0; i < mk_OutputBoxIterationKeyChooser_->count(); ++i)
		lk_Items << mk_OutputBoxIterationKeyChooser_->itemText(i);
	
	// insert all new iteration keys
	foreach (QString ls_Key, lk_IterationKeys)
	{
		if (!lk_Items.contains(ls_Key))
		{
			lk_Items << ls_Key;
		}
	}
	
	qSort(lk_Items.begin(), lk_Items.end());

	QString ls_CurrentText = mk_OutputBoxIterationKeyChooser_->currentText();
	mk_OutputBoxIterationKeyChooser_->clear();
	foreach (QString ls_Key, lk_Items)
	{
		mk_OutputBoxIterationKeyChooser_->addItem(ls_Key);
		if (!mk_Output.contains(ls_Key))
			mk_Output[ls_Key] = RefPtr<k_ConsoleString>(new k_ConsoleString());
		if (ls_Key == ls_CurrentText)
			mk_OutputBoxIterationKeyChooser_->setCurrentIndex(mk_OutputBoxIterationKeyChooser_->count() - 1);
	}

	// remove all old iteration keys
	QList<int> lk_DeleteIndices;
	for (int i = 0; i < mk_OutputBoxIterationKeyChooser_->count(); ++i)
	{
		QString ls_Key = mk_OutputBoxIterationKeyChooser_->itemText(i);
		if (!lk_IterationKeys.contains(ls_Key))
			lk_DeleteIndices << i;
	}
	for (int i = lk_DeleteIndices.size() - 1; i >= 0; --i)
	{
		QString ls_Key = mk_OutputBoxIterationKeyChooser_->itemText(lk_DeleteIndices[i]);
		mk_OutputBoxIterationKeyChooser_->removeItem(lk_DeleteIndices[i]);
		mk_Output.remove(ls_Key);
	}
}


// check whether the output directory has changed according to the input
// files. if so, update update filenames!
void k_ScriptBox::updateOutputDirectory()
{
	QString ls_OldOutputDirectory = outputDirectory();
	this->determineOutputDirectoryDefiningInputFile();
	QString ls_NewOutputDirectory = outputDirectory();
	if (ls_OldOutputDirectory != ls_NewOutputDirectory)
		this->updateOutputFilenames();
}
*/


void k_ScriptBox::proposePrefixButtonClicked(bool ab_NotifyOnFailure)
{
	// collect all input files
	QStringList lk_InputFiles;
	foreach (IDesktopBox* lk_Box_, mk_ConnectedIncomingBoxes)
	{
		IFileBox* lk_FileBox_ = dynamic_cast<IFileBox*>(lk_Box_);
		if (lk_FileBox_)
			foreach (QString ls_Path, lk_FileBox_->filenames())
				lk_InputFiles.append(ls_Path);
	}
// 		QHash<QString, QString> lk_Tags;
// 		QString ls_CommonPrefix;
// 		mk_Desktop_->createFilenameTags(lk_InputFiles, lk_Tags, ls_CommonPrefix);
// 		if ((!ls_CommonPrefix.isEmpty()) && (ls_CommonPrefix.right(1) != "-"))
// 			ls_CommonPrefix += "-";
// 		mk_Prefix.setText(ls_CommonPrefix);
	QString ls_Result = mk_pScript->proposePrefix(lk_InputFiles);
	if (!ls_Result.isEmpty())
	{
		mk_Prefix.setText(ls_Result);
		mk_Desktop_->setHasUnsavedChanges(true);
	}
	if (ls_Result.isEmpty() && ab_NotifyOnFailure)
		mk_Proteomatic.showMessageBox("Propose prefix", 
			"<p>Sorry, but Proteomatic was unable to propose a prefix.</p>", 
			":/icons/emblem-important.png", QMessageBox::Ok, QMessageBox::Ok, QMessageBox::Ok);
}


void k_ScriptBox::clearPrefixButtonClicked()
{
	mk_Prefix.setText(QString());
	mk_Desktop_->setHasUnsavedChanges(true);
	update();
}


void k_ScriptBox::clearOutputDirectoryButtonClicked()
{
	mk_OutputDirectory.setText(QString());
	mk_Desktop_->setHasUnsavedChanges(true);
	update();
}


void k_ScriptBox::start(const QString& as_IterationKey)
{
	QHash<QString, QString> lk_Parameters;

	// set output directory
	if (!this->scriptOutputDirectory().isEmpty())
		lk_Parameters["-outputDirectory"] = this->scriptOutputDirectory();

	// set output prefix
    
    QString ls_UseTag = "";
    if (batchMode())
    {
        ls_UseTag = as_IterationKey;
        if (!useShortIterationTags())
        {
            // find only batch mode input file list
            IFileBox* lk_InFileBox_ = NULL;
            foreach (IDesktopBox* lk_OtherBox_, incomingBoxes())
            {
                lk_InFileBox_ = dynamic_cast<IFileBox*>(lk_OtherBox_);
                if (lk_InFileBox_)
                    break;
            }
            if (lk_InFileBox_)
                ls_UseTag = lk_InFileBox_->filenamesForTag(as_IterationKey).first();
        }
    }
	lk_Parameters["-outputPrefix"] = mk_Prefix.text() + ls_UseTag + (ls_UseTag.isEmpty() ? "" : "-");
	
	// set output files
	foreach (QString ls_Key, mk_pScript->outputFileKeys())
		lk_Parameters["-" + ls_Key] = mk_Checkboxes[ls_Key]->checkState() == Qt::Checked ? "yes" : "no";
	
	// collect input files
    QStringList lk_InputFiles;

	QList<k_InputGroupProxyBox*> lk_ProxyBoxes;
	
	foreach (IDesktopBox* lk_Box_, mk_ConnectedIncomingBoxes)
	{
		k_InputGroupProxyBox* lk_ProxyBox_ = dynamic_cast<k_InputGroupProxyBox*>(lk_Box_);
		if (lk_ProxyBox_ && !lk_ProxyBox_->groupKey().isEmpty())
			lk_ProxyBoxes.push_back(lk_ProxyBox_);
		else
		{
			IFileBox* lk_FileBox_ = dynamic_cast<IFileBox*>(lk_Box_);
			if (lk_FileBox_)
                lk_InputFiles += lk_FileBox_->filenamesForTag(as_IterationKey);
		}
	}
	
	foreach (k_InputGroupProxyBox* lk_ProxyBox_, lk_ProxyBoxes)
	{
		QStringList lk_Files;
		foreach (IFileBox* lk_FileBox_, lk_ProxyBox_->fileBoxes())
			lk_Files += lk_FileBox_->filenamesForTag(as_IterationKey);
        
		if (lk_Files.size() > 0)
		{
			lk_InputFiles.push_back("-" + lk_ProxyBox_->groupKey());
            lk_InputFiles += lk_Files;
		}
	}
	
	ms_CurrentIterationKeyRunning = as_IterationKey;
	ms_CurrentIterationKeyShowing = as_IterationKey;
	mk_OutputBoxIterationKeyChooser_->setCurrentIndex(mk_OutputBoxIterationKeyChooser_->findText(ms_CurrentIterationKeyShowing));
	
	if (!mk_Output.contains(as_IterationKey))
		mk_Output.insert(as_IterationKey, RefPtr<k_ConsoleString>(new k_ConsoleString()));
	mk_Output[as_IterationKey]->clear();
	mk_pScript->start(lk_InputFiles, lk_Parameters, mk_Desktop_->useFileTrackerIfAvailable());
}


void k_ScriptBox::abort()
{
	mk_pScript->kill();
	addOutput("\nScript aborted by user.");
}


void k_ScriptBox::readyReadSlot()
{
	addOutput(mk_pScript->readAll());
}


void k_ScriptBox::addOutput(QString as_Text)
{
	if (mk_Output.contains(ms_CurrentIterationKeyRunning))
		mk_Output[ms_CurrentIterationKeyRunning]->append(as_Text);
	if (ms_CurrentIterationKeyRunning == ms_CurrentIterationKeyShowing)
	{
		mk_OutputBox_->setText(mk_Output[ms_CurrentIterationKeyShowing]->text());
		mk_OutputBox_->moveCursor(QTextCursor::End);
		mk_OutputBox_->ensureCursorVisible();
	}
}


void k_ScriptBox::showOutputBox(bool ab_Flag/* = true*/)
{
	mk_TabWidget_->setCurrentWidget(mk_OutputBoxContainer_);
}


void k_ScriptBox::scriptParameterChanged(const QString& as_Key)
{
	if (mk_pScript->type() == r_ScriptType::Converter)
		if (mk_ConverterFilenameAffectingParameters.contains(as_Key))
			update();
	mk_Desktop_->setHasUnsavedChanges(true);
}


void k_ScriptBox::chooseOutputDirectory()
{
	QString ls_Path = QFileDialog::getExistingDirectory(this, tr("Select output directory"), mk_OutputDirectory.text().isEmpty()? mk_Proteomatic.getConfiguration(CONFIG_REMEMBER_OUTPUT_PATH).toString(): mk_OutputDirectory.text());
	if (ls_Path.length() > 0)
	{
		mk_OutputDirectory.setText(ls_Path);
		mk_Desktop_->setHasUnsavedChanges(true);
	}
}


void k_ScriptBox::hidingBuddy()
{
	mk_LastUserAdjustedSize = this->size();
	this->setResizable(false, false);
}


void k_ScriptBox::showingBuddy()
{
	this->setResizable(true, false);
	this->resize(mk_LastUserAdjustedSize);
}


void k_ScriptBox::outputBoxIterationKeyChooserChanged()
{
	ms_CurrentIterationKeyShowing = mk_OutputBoxIterationKeyChooser_->currentText(); 
	if (mk_Output.contains(ms_CurrentIterationKeyShowing))
	{
		mk_OutputBox_->setText(mk_Output[ms_CurrentIterationKeyShowing]->text());
		mk_OutputBox_->moveCursor(QTextCursor::End);
		mk_OutputBox_->ensureCursorVisible();
	}
}


void k_ScriptBox::update()
{
	// ----------------------------------------------
	// UPDATE BATCH MODE
	// ----------------------------------------------
	
	// batch mode this box if at least one incoming box is in batch mode
	bool lb_BatchMode = false;
    IFileBox* lk_FirstBatchFileBox_ = NULL;
	// converter scripts never go into batch mode! 
	// in converter script, batch mode goes into you!!
    int li_InputBatchCount = 0;
	if (mk_pScript->type() != r_ScriptType::Converter)
	{
		foreach (IDesktopBox* lk_Box_, mk_ConnectedIncomingBoxes)
		{
			if (lk_Box_->batchMode())
			{
                if (!lk_FirstBatchFileBox_)
                    lk_FirstBatchFileBox_ = dynamic_cast<IFileBox*>(lk_Box_);
				lb_BatchMode = true;
                ++li_InputBatchCount;
                if (li_InputBatchCount > 1)
                    break;
			}
		}
	}
    mb_MultipleInputBatches = li_InputBatchCount > 1;
	setBatchMode(lb_BatchMode);
    if (mb_MultipleInputBatches)
        mk_UseShortTagsCheckBox_->setChecked(true);
    
    // ----------------------------------------------
    // UPDATE BATCH MODE ITERATION TAGS
    // ----------------------------------------------
    
    mk_IterationTags.clear();
    // determine iteration tags if in batch mode
    mb_IterationsTagsDontMatch = false;
    if (batchMode())
    {
        // collect all incoming file boxes, 
        // even those behind an input group proxy box
        QList<IFileBox*> lk_IncomingFileBoxes;
        foreach (IDesktopBox* lk_Box_, this->incomingBoxes())
        {
            k_InputGroupProxyBox* lk_InputGroupProxyBox_ = dynamic_cast<k_InputGroupProxyBox*>(lk_Box_);
            if (lk_InputGroupProxyBox_)
                lk_IncomingFileBoxes += lk_InputGroupProxyBox_->fileBoxes();
            else
            {
                IFileBox* lk_FileBox_ = dynamic_cast<IFileBox*>(lk_Box_);
                if (lk_FileBox_)
                    lk_IncomingFileBoxes << lk_FileBox_;
            }
        }
        QSet<QString> lk_ConsensusIterationTags;
        bool lb_First = true;
        foreach (IFileBox* lk_FileBox_, lk_IncomingFileBoxes)
        {
            if (dynamic_cast<IDesktopBox*>(lk_FileBox_)->batchMode())
            {
                QSet<QString> lk_IterationTags;
                foreach (QString ls_Path, lk_FileBox_->filenames())
                {
                    QString ls_Tag = lk_FileBox_->tagForFilename(ls_Path);
                    lk_IterationTags << ls_Tag;
                }
                if (lb_First)
                {
                    lk_ConsensusIterationTags = lk_IterationTags;
                    lb_First = false;
                }
                else
                {
                    if (lk_ConsensusIterationTags != lk_IterationTags)
                    {
                        // iteration tags do not match,
                        // script box is in a bad state!
                        mb_IterationsTagsDontMatch = true;
                    }
                }
            }
        }
        mk_IterationTags = lk_ConsensusIterationTags.toList();
        qSort(mk_IterationTags.begin(), mk_IterationTags.end());
    }
    
	// ----------------------------------------------
	// UPDATE INPUT FILENAMES
	// ----------------------------------------------
	
	// iterate over all incoming boxes, sort input files into right groups
	mk_InputFilesForKey.clear();
    foreach (QString ls_Key, mk_pScript->inputGroupKeys())
        mk_InputFilesForKey[ls_Key] = QStringList();
    
	foreach (IDesktopBox* lk_Box_, this->incomingBoxes())
	{
		k_InputGroupProxyBox* lk_InputGroupProxyBox_ = dynamic_cast<k_InputGroupProxyBox*>(lk_Box_);
		if (lk_InputGroupProxyBox_)
		{
            QString ls_InputKey = lk_InputGroupProxyBox_->groupKey();
            foreach (IFileBox* lk_FileBox_, lk_InputGroupProxyBox_->fileBoxes())
            {
                foreach (QString ls_Path, lk_FileBox_->filenames())
                {
                    if (ls_InputKey.isEmpty())
                        ls_InputKey = mk_pScript->inputGroupForFilename(ls_Path);
                    mk_InputFilesForKey[ls_InputKey] << ls_Path;
                }
            }
		}
        else
        {
            IFileBox* lk_FileBox_ = dynamic_cast<IFileBox*>(lk_Box_);
            if (lk_FileBox_)
            {
                foreach (QString ls_Path, lk_FileBox_->filenames())
                {
                    QString ls_InputKey = mk_pScript->inputGroupForFilename(ls_Path);
                    mk_InputFilesForKey[ls_InputKey] << ls_Path;
                }
            }
        }
	}
	
	
	// ----------------------------------------------
	// UPDATE OUTPUT DIRECTORY
	// ----------------------------------------------

	// determine file that automatically defines input directory
	ms_OutputDirectoryDefiningInputPath = QString();
    if (!mk_InputFilesForKey[mk_pScript->defaultOutputDirectoryInputGroup()].empty())
    {
        QString ls_SmallestPath = mk_InputFilesForKey[mk_pScript->defaultOutputDirectoryInputGroup()].first();
        foreach (QString ls_Path, mk_InputFilesForKey[mk_pScript->defaultOutputDirectoryInputGroup()])
            if (ls_Path < ls_SmallestPath)
                ls_SmallestPath = ls_Path;
        ms_OutputDirectoryDefiningInputPath = ls_SmallestPath;
    }
	
	// ----------------------------------------------
	// UPDATE OUTPUT FILENAMES
	// ----------------------------------------------
	
    mk_OutputFilesForIterationTag.clear();
    
	if (mk_pScript->type() == r_ScriptType::Processor)
	{
		mk_OutputFilesForKey.clear();
		foreach (QString ls_Key, mk_Checkboxes.keys())
		{
			if (mk_Checkboxes[ls_Key]->checkState() == Qt::Checked)
			{
				mk_OutputFilesForKey[ls_Key] = QStringList();
                if (batchMode())
                {
                    if (!mb_IterationsTagsDontMatch)
                    {
                        foreach (QString ls_Tag, mk_IterationTags)
                        {
                            QString ls_UseTag = ls_Tag;
                            if ((!mb_MultipleInputBatches) && lk_FirstBatchFileBox_ && (mk_UseShortTagsCheckBox_->checkState() != Qt::Checked))
                            {
                                ls_UseTag = lk_FirstBatchFileBox_->filenamesForTag(ls_Tag).first();
                                ls_UseTag = QFileInfo(ls_UseTag).baseName();
                            }
                            
                            if (!ls_UseTag.isEmpty())
                                ls_UseTag += "-";
                            
                            QString ls_Path = scriptOutputDirectory() + "/" + boxOutputPrefix() + ls_UseTag + mk_pScript->outputFileDetails(ls_Key)["filename"];
                            mk_OutputFilesForKey[ls_Key] << ls_Path;
                            mk_OutputFilesForIterationTag[ls_Tag] << ls_Path;
                        }
                    }
                }
                else
                    mk_OutputFilesForKey[ls_Key] << scriptOutputDirectory() + "/" + boxOutputPrefix() + mk_pScript->outputFileDetails(ls_Key)["filename"];
			}
		}
	}
    else if (mk_pScript->type() == r_ScriptType::Converter)
    {
        mk_OutputFilesForKey.clear();
        QString ls_ConverterKey = mk_pScript->info()["converterKey"];
        mk_OutputFilesForKey[ls_ConverterKey] = QStringList();
        QString ls_ConverterFilename = mk_pScript->info()["converterFilename"];
        foreach (QString ls_Path, mk_InputFilesForKey[ls_ConverterKey])
        {
            QString ls_OutPath = ls_ConverterFilename;
            QString ls_Basename = QFileInfo(ls_Path).baseName();
            QString ls_Extension = QFileInfo(ls_Path).completeSuffix(); 
            QString ls_Filename = QFileInfo(ls_Path).fileName(); 
            QRegExp lk_RegExp("(#\\{[a-zA-Z0-9_]+\\})", Qt::CaseSensitive, QRegExp::RegExp2);
            int li_Position = 0;
            while ((li_Position = lk_RegExp.indexIn(ls_OutPath)) != -1)
            {
                QString ls_Capture = lk_RegExp.cap(1);
                QString ls_Key = ls_Capture;
                ls_Key.replace("#{", "");
                ls_Key.replace("}", "");
                QString ls_Value = "";
                if (ls_Key == "basename")
                    ls_Value = ls_Basename;
                else if (ls_Key == "extension")
                    ls_Value = ls_Extension;
                else if (ls_Key == "filename")
                    ls_Value = ls_Filename;
                else
                {
                    // it must be a parameter!
                    ls_Value = mk_pScript->parameterValue(ls_Key);
                }
                ls_OutPath.replace(ls_Capture, ls_Value);
            }
            mk_OutputFilesForKey[ls_ConverterKey] << scriptOutputDirectory() + "/" + boxOutputPrefix() + ls_OutPath;
        }
    }
    
    // ------------------------------------------------
    // UPDATE ITERATION TAG DROP-DOWN BOX
    // ------------------------------------------------

    mk_OutputBoxIterationKeyChooserContainer_->setVisible(batchMode());
    QStringList lk_IterationKeys = iterationKeys();
    
    QStringList lk_Items;
    for (int i = 0; i < mk_OutputBoxIterationKeyChooser_->count(); ++i)
        lk_Items << mk_OutputBoxIterationKeyChooser_->itemText(i);
    
    // insert all new iteration keys
    foreach (QString ls_Key, lk_IterationKeys)
    {
        if (!lk_Items.contains(ls_Key))
            lk_Items << ls_Key;
    }
    
    qSort(lk_Items.begin(), lk_Items.end());

    QString ls_CurrentText = mk_OutputBoxIterationKeyChooser_->currentText();
    mk_OutputBoxIterationKeyChooser_->clear();
    foreach (QString ls_Key, lk_Items)
    {
        mk_OutputBoxIterationKeyChooser_->addItem(ls_Key);
        if (!mk_Output.contains(ls_Key))
            mk_Output[ls_Key] = RefPtr<k_ConsoleString>(new k_ConsoleString());
        if (ls_Key == ls_CurrentText)
            mk_OutputBoxIterationKeyChooser_->setCurrentIndex(mk_OutputBoxIterationKeyChooser_->count() - 1);
    }

    // remove all old iteration keys
    QList<int> lk_DeleteIndices;
    for (int i = 0; i < mk_OutputBoxIterationKeyChooser_->count(); ++i)
    {
        QString ls_Key = mk_OutputBoxIterationKeyChooser_->itemText(i);
        if (!lk_IterationKeys.contains(ls_Key))
            lk_DeleteIndices << i;
    }
    for (int i = lk_DeleteIndices.size() - 1; i >= 0; --i)
    {
        QString ls_Key = mk_OutputBoxIterationKeyChooser_->itemText(lk_DeleteIndices[i]);
        mk_OutputBoxIterationKeyChooser_->removeItem(lk_DeleteIndices[i]);
        mk_Output.remove(ls_Key);
    }

    toggleUi();
	emit changed();
}


void k_ScriptBox::toggleUi()
{
    k_DesktopBox::toggleUi();
    if (mk_IterationsTagsDontMatchIcon_)
        mk_IterationsTagsDontMatchIcon_->setVisible(mb_IterationsTagsDontMatch);
    if (mk_UseShortTagsCheckBox_)
    {
        mk_UseShortTagsCheckBox_->setVisible(batchMode());
        mk_UseShortTagsCheckBox_->setEnabled(!mb_MultipleInputBatches);
    }
}


/*
void k_ScriptBox::showPopupMenu()
{
	QToolButton* lk_ToolButton_ = dynamic_cast<QToolButton*>(sender());
	QPoint lk_Point = lk_ToolButton_->pos() + pos() + QPoint(0, lk_ToolButton_->height());
	mk_PopupMenu_->show();
	mk_PopupMenu_->raise();
	mk_PopupMenu_->exec(lk_Point);
}
*/


void k_ScriptBox::setupLayout()
{
	setAcceptDrops(true);
	
	QBoxLayout* lk_VLayout_;
	QBoxLayout* lk_HLayout_;
	
	// build parameter proxy widget
	
	//mk_pParameterProxyWidget->resize(500, 600);
	//mk_pParameterProxyWidget->setWindowTitle(mk_pScript->title());
	//mk_pParameterProxyWidget->setWindowIcon(QIcon(":icons/proteomatic.png"));
	
	mk_TabWidget_ = new QTabWidget(this);
    
    QPushButton * lk_FloatToolButton_ = new QPushButton(this);
    lk_FloatToolButton_->setIcon(QIcon(":/icons/preferences-system-windows.png"));
    lk_FloatToolButton_->setFlat(true);
    lk_FloatToolButton_->setIconSize(QSize(16, 16));
    mk_TabWidget_->setCornerWidget(lk_FloatToolButton_);
    connect(lk_FloatToolButton_, SIGNAL(clicked()), this, SIGNAL(togglePaneFloat()));
	
	mk_pParameterProxyWidget = RefPtr<QWidget>(mk_TabWidget_);
	

// 	QToolBar* lk_ToolBar_ = new QToolBar(mk_pParameterProxyWidget.get_Pointer());
// 	lk_ToolBar_->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
// 
// 	lk_ToolBar_->addAction(QIcon(":/icons/document-properties.png"), "Profiles", this, SLOT(showProfileManager()));
// 	lk_ToolBar_->addAction(QIcon(":/icons/edit-clear.png"), "Reset", dynamic_cast<QObject*>(mk_pScript.get_Pointer()), SLOT(reset()));
// 	QWidget* lk_StretchLabel_ = new QWidget(mk_pParameterProxyWidget.get_Pointer());
// 	lk_StretchLabel_->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum));
// 	lk_StretchLabel_->setContentsMargins(0, 0, 0, 0);
// 	lk_ToolBar_->addWidget(lk_StretchLabel_);
// 	lk_ToolBar_->addAction(QIcon(":/icons/dialog-ok.png"), "Close", mk_pParameterProxyWidget.get_Pointer(), SLOT(accept()));
// 	
// 	lk_VLayout_->addWidget(lk_ToolBar_);

	QScrollArea* lk_ScrollArea_ = new QScrollArea();
	lk_ScrollArea_->setFrameStyle(QFrame::NoFrame);
	lk_ScrollArea_->setWidget(mk_pScript->parameterWidget());
	connect(mk_Desktop_, SIGNAL(pipelineIdle(bool)), mk_pScript->parameterWidget(), SLOT(setEnabled(bool)));
	lk_ScrollArea_->setWidgetResizable(true);
	mk_TabWidget_->addTab(lk_ScrollArea_, "Parameters");

	// now comes the output box
	mk_OutputBoxContainer_ = new QWidget(this);
	mk_OutputBox_ = new QTextEdit(mk_OutputBoxContainer_);
	mk_OutputBoxIterationKeyChooser_ = new QComboBox(mk_OutputBoxContainer_);
	mk_OutputBox_->setReadOnly(true);
	mk_OutputBox_->setFont(mk_Proteomatic.consoleFont());
	
	lk_VLayout_ = new QVBoxLayout(mk_OutputBoxContainer_);
	
	mk_OutputBoxIterationKeyChooserContainer_ = new QWidget(this);
	lk_HLayout_ = new QHBoxLayout(mk_OutputBoxIterationKeyChooserContainer_);
	lk_HLayout_->setContentsMargins(0, 0, 0, 0);
	
	lk_HLayout_->addWidget(new QLabel("Iteration:", this));
	lk_HLayout_->addWidget(mk_OutputBoxIterationKeyChooser_);
	connect(mk_OutputBoxIterationKeyChooser_, SIGNAL(currentIndexChanged(int)), this, SLOT(outputBoxIterationKeyChooserChanged()));
	lk_HLayout_->setStretch(0, 0);
	lk_HLayout_->setStretch(1, 1);

	lk_VLayout_->addWidget(mk_OutputBoxIterationKeyChooserContainer_);
	lk_VLayout_->addWidget(mk_OutputBox_);
	
	mk_OutputBoxIterationKeyChooserContainer_->hide();
	
	mk_TabWidget_->addTab(mk_OutputBoxContainer_, "Output messages");
	
	// now the script box ...
	
	lk_VLayout_ = new QVBoxLayout(this);
	lk_VLayout_->setContentsMargins(11, 11, 11, 11);
	
	// script title
	QLabel* lk_ScriptTitle_ = new k_UnclickableLabel("<b>" + mk_pScript->title() + "</b>", this);
	lk_VLayout_->addWidget(lk_ScriptTitle_);

	// only do this if there are output files for this script!
	if (!((mk_pScript->type() == r_ScriptType::Processor) && mk_pScript->outputFileKeys().empty()))
	{
		// horizontal rule
		QFrame* lk_Frame_ = new QFrame(this);
		lk_Frame_->setFrameStyle(QFrame::HLine | QFrame::Plain);
		lk_Frame_->setLineWidth(1);
		lk_Frame_->setStyleSheet("color: " + QString(TANGO_ALUMINIUM_3) + ";");
		lk_VLayout_->addWidget(lk_Frame_);
		
		// buttons
		lk_HLayout_ = new QHBoxLayout();
		lk_VLayout_->addLayout(lk_HLayout_);
		
	/*	QToolButton* lk_ParametersToolButton_ = new QToolButton(this);
		lk_ParametersToolButton_->setIcon(QIcon(":/icons/preferences-system.png"));
		lk_HLayout_->addWidget(lk_ParametersToolButton_);*/

		//mk_Desktop_->pipelineMainWindow().tabWidget()->addTab(mk_pParameterProxyWidget.get_Pointer(), mk_pScript->title());
		//lk_DockWidget_->hide();
		//connect(lk_ParametersToolButton_, SIGNAL(clicked()), lk_DockWidget_, SLOT(show()));
		//mk_pParameterProxyWidget->setParent(&(mk_Desktop_->pipelineMainWindow()), Qt::Tool);
		//mk_pParameterProxyWidget->hide();

		/*
		mk_PopupMenu_ = new QMenu(this);
		foreach (QString ls_Key, mk_pScript->outputFileKeys())
		{
			QHash<QString, QString> lk_OutputFileDetails = mk_pScript->outputFileDetails(ls_Key);
			QString ls_Label = lk_OutputFileDetails["label"];
			QAction* lk_Action_ = mk_PopupMenu_->addAction(QIcon(":icons/text-x-generic.png"), "Write " + ls_Label);
			lk_Action_->setProperty("key", ls_Key);
			lk_Action_->setCheckable(true);
			//connect(lk_Action_, SIGNAL(toggled(bool)), this, SLOT(outputFileActionToggled()));
			if (lk_OutputFileDetails["default"] == "yes" || lk_OutputFileDetails["default"] == "true")
				lk_Action_->setChecked(true);
		}
		mk_PopupMenu_->addSeparator();
		mk_PopupMenu_->addAction(QIcon(":icons/folder.png"), "Set output directory");
		QToolButton* lk_OutputFilesButton_ = new QToolButton(this);
		lk_OutputFilesButton_->setText("Output files");
		//lk_OutputFilesButton_->setPopupMode(QToolButton::InstantPopup);
		//lk_OutputFilesButton_->setMenu(lk_OutputFilesMenu_);
		lk_OutputFilesButton_->setIcon(QIcon(":icons/folder.png"));
		lk_OutputFilesButton_->setToolButtonStyle(Qt::ToolButtonIconOnly);
		lk_HLayout_->addWidget(lk_OutputFilesButton_);
		//mk_Desktop_->graphicsScene().addWidget(lk_OutputFilesMenu_);
		connect(lk_OutputFilesButton_, SIGNAL(clicked()), this, SLOT(showPopupMenu()));
		*/
		
	/*	QToolButton* lk_WatchOutputButton_ = new QToolButton(this);
		lk_WatchOutputButton_->setIcon(QIcon(":/icons/utilities-terminal.png"));
		lk_HLayout_->addWidget(lk_WatchOutputButton_);*/

		// make this window auto-close on exit
	/*	mk_OutputBox.setAttribute(Qt::WA_QuitOnClose, false);
		mk_OutputBox.setWindowIcon(QIcon(":/icons/utilities-terminal.png"));
		mk_OutputBox.setWindowTitle(mk_pScript->title());
		connect(lk_WatchOutputButton_, SIGNAL(clicked()), this, SLOT(showOutputBox()));*/
		
	/*	lk_HLayout_->addStretch();*/
		
		QWidget* lk_Container_ = new QWidget(this);
        
        mk_IterationsTagsDontMatchIcon_ = new QWidget(lk_Container_);
        QBoxLayout* lk_WarningHLayout_ = new QHBoxLayout(mk_IterationsTagsDontMatchIcon_);
        
        
        QLabel* lk_WarningIcon_ = new QLabel("", mk_IterationsTagsDontMatchIcon_);
        lk_WarningIcon_->setPixmap(QPixmap(":/icons/dialog-warning.png").scaledToHeight(16, Qt::SmoothTransformation));
        lk_WarningHLayout_->addWidget(lk_WarningIcon_);
        lk_WarningHLayout_->addWidget(new QLabel("Input file batches don't match!", mk_IterationsTagsDontMatchIcon_));
        lk_WarningHLayout_->setContentsMargins(0, 0, 0, 0);
        lk_VLayout_->addWidget(mk_IterationsTagsDontMatchIcon_);
		
		k_FoldedHeader* lk_FoldedHeader_ = new k_FoldedHeader("Output files", lk_Container_, this);
		
		lk_VLayout_->addWidget(lk_FoldedHeader_);
		lk_VLayout_->addWidget(lk_Container_);
		
		connect(lk_FoldedHeader_, SIGNAL(hidingBuddy()), this, SLOT(hidingBuddy()));
		connect(lk_FoldedHeader_, SIGNAL(showingBuddy()), this, SLOT(showingBuddy()));
		
		lk_FoldedHeader_->hideBuddy();
		
		lk_VLayout_ = new QVBoxLayout(lk_Container_);
		lk_VLayout_->setContentsMargins(0, 0, 0, 0);

		lk_HLayout_ = new QHBoxLayout();
		lk_VLayout_->addLayout(lk_HLayout_);

		mk_Prefix.setHint("output file prefix");
		lk_HLayout_->addWidget(&mk_Prefix);
		connect(mk_Desktop_, SIGNAL(pipelineIdle(bool)), &mk_Prefix, SLOT(setEnabled(bool)));
		connect(&mk_Prefix, SIGNAL(textChanged(const QString&)), this, SLOT(update()));

		QToolButton* lk_ClearPrefixButton_ = new QToolButton(this);
		lk_ClearPrefixButton_->setIcon(QIcon(":/icons/dialog-cancel.png"));
		lk_HLayout_->addWidget(lk_ClearPrefixButton_);
		connect(mk_Desktop_, SIGNAL(pipelineIdle(bool)), lk_ClearPrefixButton_, SLOT(setEnabled(bool)));
		connect(lk_ClearPrefixButton_, SIGNAL(clicked()), this, SLOT(clearPrefixButtonClicked()));
		if (mk_pScript->type() != r_ScriptType::Converter)
		{
			QToolButton* lk_ProposePrefixButton_ = new QToolButton(this);
			lk_ProposePrefixButton_->setIcon(QIcon(":/icons/select-continuous-area.png"));
			lk_HLayout_->addWidget(lk_ProposePrefixButton_);
			connect(mk_Desktop_, SIGNAL(pipelineIdle(bool)), lk_ProposePrefixButton_, SLOT(setEnabled(bool)));
			connect(lk_ProposePrefixButton_, SIGNAL(clicked()), this, SLOT(proposePrefixButtonClicked()));
		}

		lk_HLayout_ = new QHBoxLayout();
		lk_VLayout_->addLayout(lk_HLayout_);

		connect(mk_Desktop_, SIGNAL(pipelineIdle(bool)), &mk_OutputDirectory, SLOT(setEnabled(bool)));
		mk_OutputDirectory.setHint("output directory");
		mk_OutputDirectory.setReadOnly(true);
        mk_OutputDirectory.setMinimumWidth(120);
		lk_HLayout_->addWidget(&mk_OutputDirectory);
		connect(&mk_OutputDirectory, SIGNAL(textChanged(const QString&)), this, SLOT(update()));

		QToolButton* lk_ClearOutputDirectoryButton_ = new QToolButton(this);
		lk_ClearOutputDirectoryButton_->setIcon(QIcon(":/icons/dialog-cancel.png"));
		lk_HLayout_->addWidget(lk_ClearOutputDirectoryButton_);
		connect(mk_Desktop_, SIGNAL(pipelineIdle(bool)), lk_ClearOutputDirectoryButton_, SLOT(setEnabled(bool)));
		connect(lk_ClearOutputDirectoryButton_, SIGNAL(clicked()), this, SLOT(clearOutputDirectoryButtonClicked()));
		QToolButton* lk_SelectOutputDirectory_ = new QToolButton(lk_Container_);
		connect(mk_Desktop_, SIGNAL(pipelineIdle(bool)), lk_SelectOutputDirectory_, SLOT(setEnabled(bool)));
		lk_SelectOutputDirectory_->setIcon(QIcon(":/icons/folder.png"));
		lk_HLayout_->addWidget(lk_SelectOutputDirectory_);
		connect(lk_SelectOutputDirectory_, SIGNAL(clicked()), this, SLOT(chooseOutputDirectory()));
		//connect(lk_ProposePrefixButton_, SIGNAL(clicked()), this, SLOT(proposePrefixButtonClicked()));
		
		/*
		// horizontal rule
		lk_Frame_ = new QFrame(this);
		lk_Frame_->setFrameStyle(QFrame::HLine | QFrame::Plain);
		lk_Frame_->setLineWidth(1);
		lk_Frame_->setStyleSheet("color: " + QString(TANGO_ALUMINIUM_3) + ";");
		lk_VLayout_->addWidget(lk_Frame_);
		*/

        mk_UseShortTagsCheckBox_ = new QCheckBox("Use short batch prefixes", lk_Container_);
        connect(mk_UseShortTagsCheckBox_, SIGNAL(toggled(bool)), this, SLOT(update()));
        lk_VLayout_->addWidget(mk_UseShortTagsCheckBox_);
        mk_UseShortTagsCheckBox_->setChecked(true);
        
		// output file checkboxes
		foreach (QString ls_Key, mk_pScript->outputFileKeys())
		{
			QHash<QString, QString> lk_OutputFileDetails = mk_pScript->outputFileDetails(ls_Key);
			QCheckBox* lk_CheckBox_ = new QCheckBox("Write " + lk_OutputFileDetails["label"], lk_Container_);
			lk_CheckBox_->setProperty("key", ls_Key);
			connect(mk_Desktop_, SIGNAL(pipelineIdle(bool)), lk_CheckBox_, SLOT(setEnabled(bool)));
			connect(lk_CheckBox_, SIGNAL(toggled(bool)), this, SLOT(outputFileActionToggled()));
			lk_VLayout_->addWidget(lk_CheckBox_);
			mk_Checkboxes[ls_Key] = lk_CheckBox_;
			if (lk_OutputFileDetails["default"] == "yes" || lk_OutputFileDetails["default"] == "true")
				lk_CheckBox_->setChecked(true);
		}
		
		if (mk_pScript->type() == r_ScriptType::Converter)
		{
			QString ls_Key = "key";
			QString ls_Label = "Output files";
			if (mk_pScript->info().contains("converterKey"))
				ls_Key = mk_pScript->info()["converterKey"];
			if (mk_pScript->info().contains("converterLabel"))
				ls_Label = mk_pScript->info()["converterLabel"];
			ms_ConverterFilenamePattern = mk_pScript->info()["converterFilename"];
			IDesktopBox* lk_Box_ = 
				k_DesktopBoxFactory::makeOutFileListBox(mk_Desktop_, mk_Proteomatic, ls_Key, ls_Label, false);
			lk_Box_->setProtectedFromUserDeletion(true);
			dynamic_cast<QObject*>(lk_Box_)->setProperty("key", ls_Key);
			mk_OutputFileBoxes[ls_Key] = lk_Box_;
			mk_Desktop_->addBox(lk_Box_, false);
			mk_Desktop_->connectBoxes(this, lk_Box_);
			// make the output file box of a converter script a list
			dynamic_cast<k_OutFileListBox*>(lk_Box_)->setListMode(mk_pScript->type() == r_ScriptType::Converter || batchMode());
			
			// make dummy invisible checkbox
			QCheckBox* lk_CheckBox_ = new QCheckBox("dummy", lk_Container_);
			lk_CheckBox_->hide();
			lk_CheckBox_->setProperty("key", ls_Key);
			mk_Checkboxes[ls_Key] = lk_CheckBox_;
			lk_CheckBox_->setChecked(true);
            
			// collect parameters that affect the output filename
			QString ls_DestinationFilename = ms_ConverterFilenamePattern;
			QRegExp lk_RegExp("(#\\{[a-zA-Z0-9_]+\\})", Qt::CaseSensitive, QRegExp::RegExp2);
			int li_Position = 0;
			while ((li_Position = lk_RegExp.indexIn(ls_DestinationFilename)) != -1)
			{
				QString ls_Capture = lk_RegExp.cap(1);
				QString ls_Key = ls_Capture;
				ls_Key.replace("#{", "");
				ls_Key.replace("}", "");
				mk_ConverterFilenameAffectingParameters.insert(ls_Key);
				ls_DestinationFilename.replace(ls_Capture, "");
			}
			
			connect(dynamic_cast<QObject*>(mk_pScript.get_Pointer()), SIGNAL(parameterChanged(const QString&)), this, SLOT(scriptParameterChanged(const QString&)));

		}
		lk_VLayout_->addStretch();
	}
	
	mk_LastUserAdjustedSize = QSize(0, 0);
    update();
}


void k_ScriptBox::dragEnterEvent(QDragEnterEvent* event)
{
	event->acceptProposedAction();
}


void k_ScriptBox::dragMoveEvent(QDragMoveEvent* event)
{
	event->acceptProposedAction();
}


void k_ScriptBox::dropEvent(QDropEvent* event)
{
	event->accept();
	foreach (QUrl lk_Url, event->mimeData()->urls())
	{
		QString ls_Path = lk_Url.toLocalFile();
		if (!ls_Path.isEmpty())
		{
			if (QFileInfo(ls_Path).isDir())
				mk_OutputDirectory.setText(ls_Path);
		}
	}
}


Qt::DropActions k_ScriptBox::supportedDropActions() const
{
	return Qt::ActionMask;
}