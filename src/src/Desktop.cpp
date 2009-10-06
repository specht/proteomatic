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

#include "Desktop.h"
#include "DesktopBoxFactory.h"
#include "DesktopBox.h"
#include "FileListBox.h"
#include "IFileBox.h"
#include "InputGroupProxyBox.h"
#include "IScriptBox.h"
#include "ScriptBox.h"
#include "PipelineMainWindow.h"
#include "Tango.h"
#include "Yaml.h"


k_Desktop::k_Desktop(QWidget* ak_Parent_, k_Proteomatic& ak_Proteomatic, k_PipelineMainWindow& ak_PipelineMainWindow)
	: QGraphicsView(ak_Parent_)
	, mk_Proteomatic(ak_Proteomatic)
	, mk_PipelineMainWindow(ak_PipelineMainWindow)
	, mk_GraphicsScene(ak_Parent_)
	, md_Scale(1.0)
	, mk_ArrowStartBox_(NULL)
	, mk_ArrowEndBox_(NULL)
	, mk_UserArrowPathItem_(NULL)
	, mk_ArrowStartBoxAutoConnect_(NULL)
	, mb_Running(false)
	, mb_Error(false)
	, mk_CurrentScriptBox_(NULL)
	, md_BoxZ(0.0)
	, mb_HasUnsavedChanges(false)
	, mb_Moving(false)
	, mb_UseFileTrackerIfAvailable(true)
{
	connect(&mk_PipelineMainWindow, SIGNAL(forceRefresh()), this, SLOT(refresh()));
	connect(this, SIGNAL(showAllRequested()), this, SLOT(showAll()));
	setAcceptDrops(true);
	setTransformationAnchor(QGraphicsView::AnchorViewCenter);
	setScene(&mk_GraphicsScene);
	setRenderHint(QPainter::Antialiasing, true);
	setRenderHint(QPainter::TextAntialiasing, true);
	setRenderHint(QPainter::SmoothPixmapTransform, true);
	setBackgroundBrush(QBrush(QColor("#f8f8f8")));
	setSceneRect(0.0, 0.0, 20000.0, 20000.0);
	setDragMode(QGraphicsView::ScrollHandDrag);
	setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
	centerOn(10000.5, 10000.5);
	
	QPen lk_Pen(QColor(TANGO_ALUMINIUM_3));
	lk_Pen.setWidthF(1.5);
	lk_Pen.setStyle(Qt::DashLine);
	mk_SelectionGraphicsPathItem_ = mk_GraphicsScene.addPath(QPainterPath(), lk_Pen);
	mk_SelectionGraphicsPathItem_->setZValue(-4.0);
	
	lk_Pen.setStyle(Qt::SolidLine);
	lk_Pen.setColor(QColor(TANGO_SCARLET_RED_2));
	mk_CurrentScriptBoxGraphicsPathItem_ = mk_GraphicsScene.addPath(QPainterPath(), lk_Pen);
	mk_CurrentScriptBoxGraphicsPathItem_->setZValue(-2.0);

	lk_Pen = QPen(QColor(TANGO_BUTTER_2));
	lk_Pen.setWidthF(1.5);
	mk_BatchGraphicsPathItem_ = mk_GraphicsScene.addPath(QPainterPath(), lk_Pen, QBrush(QColor(TANGO_BUTTER_0)));
	mk_BatchGraphicsPathItem_->setZValue(-3.0);
}


k_Desktop::~k_Desktop()
{
	while (!mk_Boxes.empty())
		removeBox(mk_Boxes.toList().first());
}


k_PipelineMainWindow& k_Desktop::pipelineMainWindow() const
{
	return mk_PipelineMainWindow;
}


QGraphicsScene& k_Desktop::graphicsScene()
{
	return mk_GraphicsScene;
}

	
IDesktopBox* k_Desktop::addInputFileListBox()
{
	IDesktopBox* lk_Box_ = k_DesktopBoxFactory::makeFileListBox(this, mk_Proteomatic);
	k_DesktopBox* lk_DesktopBox_ = dynamic_cast<k_DesktopBox*>(lk_Box_);
	addBox(lk_Box_);
	lk_DesktopBox_->resize(300, 10);
	return lk_DesktopBox_;
}


IDesktopBox* k_Desktop::addScriptBox(const QString& as_ScriptUri)
{
	IDesktopBox* lk_Box_ = k_DesktopBoxFactory::makeScriptBox(as_ScriptUri, this, mk_Proteomatic);
	if (lk_Box_)
	{
		IScriptBox* lk_ScriptBox_ = dynamic_cast<IScriptBox*>(lk_Box_);
		mk_BoxForScript[lk_ScriptBox_->script()] = lk_ScriptBox_;
		addBox(lk_Box_);
		connect(dynamic_cast<QObject*>(lk_ScriptBox_), SIGNAL(scriptStarted()), this, SLOT(scriptStarted()));
		connect(dynamic_cast<QObject*>(lk_ScriptBox_), SIGNAL(scriptFinished(int)), this, SLOT(scriptFinished(int)));
		setCurrentScriptBox(lk_ScriptBox_);
		mb_Error = false;
		QSet<QString> lk_InputGroups = lk_ScriptBox_->script()->inputGroupKeys().toSet();
		foreach (QString ls_GroupKey, lk_ScriptBox_->script()->ambiguousInputGroups())
		{
			QString ls_GroupLabel = lk_ScriptBox_->script()->inputGroupLabel(ls_GroupKey);
			IDesktopBox* lk_ProxyBox_ = k_DesktopBoxFactory::makeInputGroupProxyBox(this, mk_Proteomatic, ls_GroupLabel, ls_GroupKey);
			if (lk_ProxyBox_)
			{
				addBox(lk_ProxyBox_);
				connectBoxes(lk_ProxyBox_, lk_Box_);
			}
			lk_InputGroups.remove(ls_GroupKey);
		}
		if (!lk_ScriptBox_->script()->ambiguousInputGroups().empty() && !lk_InputGroups.empty())
		{
			// add another 'remaining files' input proxy box and disallow direct connections to the script box
			IDesktopBox* lk_ProxyBox_ = k_DesktopBoxFactory::makeInputGroupProxyBox(this, mk_Proteomatic, "Remaining input files", "");
			if (lk_ProxyBox_)
			{
				addBox(lk_ProxyBox_);
				connectBoxes(lk_ProxyBox_, lk_Box_);
			}
		}
		if (mk_ArrowStartBoxAutoConnect_)
		{
			if (connectionAllowed(mk_ArrowStartBoxAutoConnect_, lk_Box_))
				connectBoxes(mk_ArrowStartBoxAutoConnect_, lk_Box_);
			k_DesktopBox* lk_ScriptDesktopBox_ = dynamic_cast<k_DesktopBox*>(lk_ScriptBox_);
			if (lk_ScriptDesktopBox_)
			{
				QSize lk_Size = lk_ScriptDesktopBox_->size();
				double ld_Length = sqrt(mk_ArrowDirection.x() * mk_ArrowDirection.x() + mk_ArrowDirection.y() * mk_ArrowDirection.y());
				mk_ArrowDirection /= ld_Length;
				lk_Size.setWidth(lk_Size.width() * (1.0 - mk_ArrowDirection.x()) * 0.5 + 0.5);
				lk_Size.setHeight(lk_Size.height() * (1.0 - mk_ArrowDirection.x()) * 0.5 + 0.5);
				lk_ScriptDesktopBox_->move(mk_ArrowEndPoint.toPoint() - QPoint(lk_Size.width(), lk_Size.height()));
			}
		}
	}
	mk_ArrowStartBoxAutoConnect_ = NULL;
	return lk_Box_;
}


void k_Desktop::addBox(IDesktopBox* ak_Box_)
{
	k_DesktopBox* lk_DesktopBox_ = dynamic_cast<k_DesktopBox*>(ak_Box_);
	QRectF lk_BoundingRect = mk_GraphicsScene.itemsBoundingRect();
	mk_ProxyWidgetForBox[lk_DesktopBox_] = mk_GraphicsScene.addWidget(lk_DesktopBox_);
	IFileBox* lk_FileBox_ = dynamic_cast<IFileBox*>(ak_Box_);
	if (lk_FileBox_)
	{
		connect(dynamic_cast<QObject*>(lk_FileBox_), SIGNAL(arrowPressed()), this, SLOT(arrowPressed()));
		connect(dynamic_cast<QObject*>(lk_FileBox_), SIGNAL(arrowReleased()), this, SLOT(arrowReleased()));
	}
	connect(lk_DesktopBox_, SIGNAL(moved(QPoint)), this, SLOT(boxMovedOrResized(QPoint)));
	connect(lk_DesktopBox_, SIGNAL(resized()), this, SLOT(boxMovedOrResized()));
	connect(lk_DesktopBox_, SIGNAL(clicked(QMouseEvent*)), this, SLOT(boxClicked(QMouseEvent*)));
	connect(lk_DesktopBox_, SIGNAL(batchModeChanged(bool)), this, SLOT(boxBatchModeChanged(bool)));
	mk_Boxes.insert(ak_Box_);
	lk_DesktopBox_->resize(1, 1);
	QPointF lk_FreeSpace = findFreeSpace(lk_BoundingRect, mk_Boxes.size() - 1, ak_Box_);
	lk_DesktopBox_->move(QPoint((int)lk_FreeSpace.x(), (int)lk_FreeSpace.y()));
	redraw();
	mb_HasUnsavedChanges = true;
	mk_PipelineMainWindow.toggleUi();
}


void k_Desktop::removeBox(IDesktopBox* ak_Box_)
{
	if (!mk_Boxes.contains(ak_Box_))
		return;

	if (mk_DeleteBoxStackSet.contains(ak_Box_))
		return;
	mk_DeleteBoxStackSet.insert(ak_Box_);

	IScriptBox* lk_ScriptBox_ = dynamic_cast<IScriptBox*>(ak_Box_);
	
	// explicitely delete incoming input group proxy boxes if this is a script box
	if (lk_ScriptBox_)
		foreach (IDesktopBox* lk_Box_, ak_Box_->incomingBoxes())
		{
			k_InputGroupProxyBox* lk_ProxyBox_ = dynamic_cast<k_InputGroupProxyBox*>(lk_Box_);
			if (lk_ProxyBox_)
				this->removeBox(lk_ProxyBox_);
		}
	
	foreach (IDesktopBox* lk_Box_, ak_Box_->incomingBoxes())
		disconnectBoxes(lk_Box_, ak_Box_);
	foreach (IDesktopBox* lk_Box_, ak_Box_->outgoingBoxes())
		disconnectBoxes(ak_Box_, lk_Box_);
	
	if (lk_ScriptBox_)
		mk_BoxForScript.remove(lk_ScriptBox_->script());
	
	mk_ProxyWidgetForBox.remove(ak_Box_);
	
	if (ak_Box_)
		delete ak_Box_;
	mk_Boxes.remove(ak_Box_);
	mk_SelectedBoxes.remove(ak_Box_);
	mk_BatchBoxes.remove(ak_Box_);
	if (mk_CurrentScriptBox_ == ak_Box_)
		setCurrentScriptBox(NULL);
	
	redrawSelection();
	redrawBatchFrame();
	mb_HasUnsavedChanges = true;
	mk_PipelineMainWindow.toggleUi();
	mk_DeleteBoxStackSet.remove(ak_Box_);
	emit selectionChanged();
}


void k_Desktop::connectBoxes(IDesktopBox* ak_Source_, IDesktopBox* ak_Destination_)
{
	tk_BoxPair lk_BoxPair(ak_Source_, ak_Destination_);
	if (mk_ArrowForBoxPair.contains(lk_BoxPair))
		return;
	ak_Source_->connectOutgoingBox(ak_Destination_);
	QGraphicsPathItem* lk_GraphicsPathItem_ = 
		mk_GraphicsScene.addPath(QPainterPath(), QPen(QColor(TANGO_ALUMINIUM_3)), QBrush(QColor(TANGO_ALUMINIUM_3)));
	lk_GraphicsPathItem_->setZValue(-1.0);
	QPen lk_Pen("#f8f8f8");
	lk_Pen.setWidthF(10.0);
	QGraphicsLineItem* lk_ArrowProxy_ = 
		mk_GraphicsScene.addLine(QLineF(), lk_Pen);
	lk_ArrowProxy_->setZValue(-1000.0);
	mk_ArrowForProxy[lk_ArrowProxy_] = lk_GraphicsPathItem_;
	mk_Arrows[lk_GraphicsPathItem_] = tk_BoxPair(ak_Source_, ak_Destination_);
	mk_ArrowForBoxPair[lk_BoxPair] = lk_GraphicsPathItem_;
	mk_ArrowsForBox[ak_Source_].insert(lk_GraphicsPathItem_);
	mk_ArrowsForBox[ak_Destination_].insert(lk_GraphicsPathItem_);
	mk_ArrowProxy[lk_GraphicsPathItem_] = lk_ArrowProxy_;
	this->updateArrow(lk_GraphicsPathItem_);
	mb_HasUnsavedChanges = true;
	mk_PipelineMainWindow.toggleUi();
}


void k_Desktop::disconnectBoxes(IDesktopBox* ak_Source_, IDesktopBox* ak_Destination_)
{
	tk_BoxPair lk_BoxPair(ak_Source_, ak_Destination_);
	if (!mk_ArrowForBoxPair.contains(lk_BoxPair))
		return;
	QGraphicsPathItem* lk_GraphicsPathItem_ = mk_ArrowForBoxPair[lk_BoxPair];
	mk_Arrows.remove(lk_GraphicsPathItem_);
	mk_ArrowForBoxPair.remove(lk_BoxPair);
	mk_ArrowsForBox[ak_Source_].remove(lk_GraphicsPathItem_);
	mk_ArrowsForBox[ak_Destination_].remove(lk_GraphicsPathItem_);
	mk_ArrowForProxy.remove(mk_ArrowProxy[lk_GraphicsPathItem_]);
	delete mk_ArrowProxy[lk_GraphicsPathItem_];
	mk_ArrowProxy.remove(lk_GraphicsPathItem_);
	delete lk_GraphicsPathItem_;
	ak_Source_->disconnectOutgoingBox(ak_Destination_);
	mb_HasUnsavedChanges = true;
	mk_PipelineMainWindow.toggleUi();
}


void k_Desktop::moveSelectedBoxesStart(IDesktopBox* ak_IncludeThis_)
{
	mk_MoveSelectionStartPositions.clear();
	QSet<IDesktopBox*> lk_SelectedBoxes = mk_SelectedBoxes;
	if (ak_IncludeThis_)
		lk_SelectedBoxes.insert(ak_IncludeThis_);
	foreach (IDesktopBox* lk_Box_, lk_SelectedBoxes)
	{
		k_DesktopBox* lk_DesktopBox_ = dynamic_cast<k_DesktopBox*>(lk_Box_);
		if (lk_DesktopBox_)
			mk_MoveSelectionStartPositions[lk_Box_] = lk_DesktopBox_->pos();
	}
	mb_Moving = true;
}


void k_Desktop::moveSelectedBoxes(QPointF ak_Delta)
{
	foreach (IDesktopBox* lk_Box_, mk_SelectedBoxes)
	{
		k_DesktopBox* lk_DesktopBox_ = dynamic_cast<k_DesktopBox*>(lk_Box_);
		if (lk_DesktopBox_ && mk_MoveSelectionStartPositions.contains(lk_Box_))
			lk_DesktopBox_->move(mk_MoveSelectionStartPositions[lk_Box_] + ak_Delta.toPoint());
	}
}


void k_Desktop::createFilenameTags(QStringList ak_Filenames, QHash<QString, QString>& ak_TagForFilename, QString& as_PrefixWithoutTags)
{
	QHash<QString, QString> lk_Result;
	QStringList lk_Files = ak_Filenames;
	qSort(lk_Files.begin(), lk_Files.end());
	QHash<QString, QSet<QString> > lk_TagInFilenames;
	QHash<QString, int> lk_MinTagCountInFilenames;
	QHash<QString, QStringList> lk_AllChopped;
	foreach (QString ls_Path, lk_Files)
	{
		QString ls_Basename = QFileInfo(ls_Path).baseName();
		// ls_Basename contains MT_Hyd_CPAN_040708_33-no-ms1
		QStringList lk_Chopped;
		// chop string
		int li_LastClass = -1;
		for (int i = 0; i < ls_Basename.length(); ++i)
		{
			int li_ThisClass = 0;
			QChar lk_Char = ls_Basename.at(i);
			if (lk_Char >= '0' && lk_Char <= '9')
				li_ThisClass = 1;
			else if ((lk_Char >= 'A' && lk_Char <= 'Z') || (lk_Char >= 'a' && lk_Char <= 'z'))
				li_ThisClass = 2;
			if (li_ThisClass != li_LastClass)
				lk_Chopped.append(lk_Char);
			else
				lk_Chopped.last().append(lk_Char);
			li_LastClass = li_ThisClass;
		}
		lk_AllChopped[ls_Path] = lk_Chopped;
		// lk_Chopped contains: MT _ Hyd _ CPAN _ 040708 _ 33 - no - ms 1
		QHash<QString, int> lk_TagCount;
		foreach (QString ls_Tag, lk_Chopped)
		{
			if (!lk_TagCount.contains(ls_Tag))
				lk_TagCount[ls_Tag] = 0;
			lk_TagCount[ls_Tag] += 1;
		}
		foreach (QString ls_Tag, lk_TagCount.keys())
		{
			if (!lk_TagInFilenames.contains(ls_Tag))
				lk_TagInFilenames[ls_Tag] = QSet<QString>();
			lk_TagInFilenames[ls_Tag].insert(ls_Path);
			if (!lk_MinTagCountInFilenames.contains(ls_Tag))
				lk_MinTagCountInFilenames[ls_Tag] = lk_TagCount[ls_Tag];
			lk_MinTagCountInFilenames[ls_Tag] = std::min<int>(lk_MinTagCountInFilenames[ls_Tag], lk_TagCount[ls_Tag]);
		}
	}
	foreach (QString ls_Path, lk_Files)
	{
		QStringList lk_Chopped = lk_AllChopped[ls_Path];
		foreach (QString ls_Tag, lk_TagInFilenames.keys())
		{
			if (lk_TagInFilenames[ls_Tag].size() != lk_Files.size())
				continue;
			for (int i = 0; i < lk_MinTagCountInFilenames[ls_Tag]; ++i)
				lk_Chopped.removeOne(ls_Tag);
		}
		QString ls_Short = lk_Chopped.join("");
		lk_Result[ls_Path] = ls_Short;
	}
	
	// determine common prefix without tags
	QString ls_PrefixWithoutTags;
	if (!lk_Files.empty())
	{
		QStringList lk_Chopped = lk_AllChopped[lk_Files.first()];
		foreach (QString ls_Tag, lk_Chopped)
		{
			if (lk_TagInFilenames[ls_Tag].size() != lk_Files.size())
				continue;
			if (lk_MinTagCountInFilenames[ls_Tag] > 0)
			{
				ls_PrefixWithoutTags += ls_Tag;
				lk_MinTagCountInFilenames[ls_Tag] -= 1;
			}
		}
	}
	
	ak_TagForFilename = lk_Result;
	as_PrefixWithoutTags = ls_PrefixWithoutTags;
}


bool k_Desktop::running() const
{
	return mb_Running;
}


bool k_Desktop::hasBoxes()
{
	return !mk_Boxes.empty();
}


tk_YamlMap k_Desktop::pipelineDescription()
{
	// collect script boxes
	QSet<tk_BoxPair> lk_UnsavedArrows = mk_Arrows.values().toSet();
	
	tk_YamlMap lk_Description;
	tk_YamlSequence lk_ScriptBoxes;
	foreach (IDesktopBox* lk_Box_, mk_Boxes)
	{
		IScriptBox* lk_ScriptBox_ = dynamic_cast<IScriptBox*>(lk_Box_);
		if (lk_ScriptBox_)
		{
			tk_YamlMap lk_ScriptBoxDescription;
			lk_ScriptBoxDescription["id"] = (qint64)lk_Box_;
			lk_ScriptBoxDescription["uri"] = QFileInfo(lk_ScriptBox_->script()->uri()).fileName();
			tk_YamlMap lk_ScriptParameters;
			QHash<QString, QString> lk_Hash = lk_ScriptBox_->script()->configuration();
			QHash<QString, QString>::const_iterator lk_Iter = lk_Hash.begin();
			for (; lk_Iter != lk_Hash.end(); ++lk_Iter)
				lk_ScriptParameters[lk_Iter.key()] = lk_Iter.value();
			lk_ScriptBoxDescription["parameters"] = lk_ScriptParameters;
			tk_YamlSequence lk_Coordinates;
			lk_Coordinates.push_back(boxLocation(lk_Box_).x() - 10000);
			lk_Coordinates.push_back(boxLocation(lk_Box_).y() - 10000);
			lk_ScriptBoxDescription["position"] = lk_Coordinates;
/*			lk_ScriptBoxDescription["outputPrefix"] = lk_ScriptBox_->boxOutputPrefix();
			lk_ScriptBoxDescription["outputDirectory"] = lk_ScriptBox_->boxOutputDirectory();*/
			// now add active output boxes
			tk_YamlMap lk_ActiveOutputFileBoxes;
			foreach (QString ls_Key, lk_ScriptBox_->script()->outputFileKeys())
			{
				if (lk_ScriptBox_->outputFileActivated(ls_Key))
				{
					IDesktopBox* lk_OutputFileBox_ = lk_ScriptBox_->boxForOutputFileKey(ls_Key);
					tk_YamlMap lk_Map;
					tk_YamlSequence lk_Coordinates;
					lk_Coordinates.push_back(boxLocation(lk_OutputFileBox_).x() - 10000);
					lk_Coordinates.push_back(boxLocation(lk_OutputFileBox_).y() - 10000);
					lk_Map["position"] = lk_Coordinates;
					lk_Map["id"] = (qint64)lk_OutputFileBox_;
					lk_ActiveOutputFileBoxes[ls_Key] = lk_Map;
					lk_UnsavedArrows.remove(tk_BoxPair(lk_Box_, lk_OutputFileBox_));
				}
			}
			lk_ScriptBoxDescription["activeOutputFiles"] = lk_ActiveOutputFileBoxes;
			if (lk_ScriptBox_->script()->type() == r_ScriptType::Converter)
			{
				IDesktopBox* lk_OtherBox_ = lk_Box_->outgoingBoxes().toList().first();
				tk_YamlSequence lk_Position;
				lk_Position.push_back(boxLocation(lk_OtherBox_).x() - 10000);
				lk_Position.push_back(boxLocation(lk_OtherBox_).y() - 10000);
				lk_ScriptBoxDescription["converterOutputFileBoxPosition"] = lk_Position;
				lk_ScriptBoxDescription["converterOutputFileBoxId"] = (qint64)lk_OtherBox_;
			}
			lk_ScriptBoxes.push_back(lk_ScriptBoxDescription);
		}
	}
	lk_Description["scriptBoxes"] = lk_ScriptBoxes;

	// now come the input file boxes
	tk_YamlSequence lk_InputFileListBoxes;
	foreach (IDesktopBox* lk_Box_, mk_Boxes)
	{
		k_FileListBox* lk_FileListBox_ = dynamic_cast<k_FileListBox*>(lk_Box_);
		if (lk_FileListBox_)
		{
			tk_YamlMap lk_BoxDescription;
			lk_BoxDescription["id"] = (qint64)lk_Box_;
			tk_YamlSequence lk_Coordinates;
			lk_Coordinates.push_back(boxLocation(lk_Box_).x() - 10000);
			lk_Coordinates.push_back(boxLocation(lk_Box_).y() - 10000);
			lk_BoxDescription["position"] = lk_Coordinates;
			tk_YamlSequence lk_Paths;
			foreach (QString ls_Path, lk_FileListBox_->filenames())
				lk_Paths.push_back(ls_Path);
			lk_BoxDescription["paths"] = lk_Paths;
			lk_InputFileListBoxes.push_back(lk_BoxDescription);
		}
	}
	lk_Description["inputFileListBoxes"] = lk_InputFileListBoxes;
	
	// continue with with remaining arrows
	tk_YamlSequence lk_Arrows;
	foreach (tk_BoxPair lk_BoxPair, lk_UnsavedArrows)
	{
		// skip this arrow if it comes from a converter script box, because in that case,
		// the arrow is already there by definition
		IScriptBox* lk_ScriptBox_ = dynamic_cast<IScriptBox*>(lk_BoxPair.first);
		if (lk_ScriptBox_ && lk_ScriptBox_->script()->type() == r_ScriptType::Converter)
			continue;
		
		tk_YamlSequence lk_Pair;
		lk_Pair.push_back((qint64)lk_BoxPair.first);
		lk_Pair.push_back((qint64)lk_BoxPair.second);
		lk_Arrows.push_back(lk_Pair);
	}
	lk_Description["connections"] = lk_Arrows;
	
	return lk_Description;
}


void k_Desktop::applyPipelineDescription(tk_YamlMap ak_Description)
{
	clearAll();
	QHash<QString, IDesktopBox*> lk_BoxForId;
	foreach (QVariant lk_Item, ak_Description["scriptBoxes"].toList())
	{
		tk_YamlMap lk_BoxDescription = lk_Item.toMap();
		QString ls_Uri = lk_BoxDescription["uri"].toString();
		QString ls_Id = lk_BoxDescription["id"].toString();
		QString ls_CompleteUri = QFileInfo(QDir(mk_Proteomatic.scriptPathAndPackage()), ls_Uri).absoluteFilePath();
		tk_YamlMap lk_OutputBoxes = lk_BoxDescription["activeOutputFiles"].toMap();
		IDesktopBox* lk_Box_ = addScriptBox(ls_CompleteUri);
		if (!lk_Box_)
		{
			// loading a script failed, now cancel this whole thing
			QString ls_Title = ls_Uri;
			mk_Proteomatic.showMessageBox("Error", 
				QString("While trying to load the pipeline, the script '%1' could not be loaded.").arg(ls_Title),
				":icons/dialog-warning.png");
			clearAll();
			setHasUnsavedChanges(false);
			return;
		}
		if (lk_Box_)
		{
			IScriptBox* lk_ScriptBox_ = dynamic_cast<IScriptBox*>(lk_Box_);
			if (lk_ScriptBox_)
			{
				lk_BoxForId[ls_Id] = lk_Box_;
				tk_YamlMap lk_Parameters = lk_BoxDescription["parameters"].toMap();
				foreach (QString ls_Key, lk_Parameters.keys())
					lk_ScriptBox_->script()->setParameter(ls_Key, lk_Parameters[ls_Key].toString());
				tk_YamlSequence lk_Position = lk_BoxDescription["position"].toList();
				moveBoxTo(lk_Box_, QPoint(lk_Position[0].toInt() + 10000, lk_Position[1].toInt() + 10000));
				foreach (QString ls_Key, lk_ScriptBox_->script()->outputFileKeys())
				{
					lk_ScriptBox_->setOutputFileActivated(ls_Key, lk_OutputBoxes.contains(ls_Key));
					if (lk_OutputBoxes.contains(ls_Key))
					{
						tk_YamlMap lk_OutBoxDescription = lk_OutputBoxes[ls_Key].toMap();
						tk_YamlSequence lk_Position = lk_OutBoxDescription["position"].toList();
						lk_BoxForId[lk_OutBoxDescription["id"].toString()] = lk_ScriptBox_->boxForOutputFileKey(ls_Key);
						moveBoxTo(lk_ScriptBox_->boxForOutputFileKey(ls_Key), QPoint(lk_Position[0].toInt() + 10000, lk_Position[1].toInt() + 10000));
					}
				}
				if (lk_ScriptBox_->script()->type() == r_ScriptType::Converter && lk_BoxDescription.contains("converterOutputFileBoxPosition"))
				{
					tk_YamlSequence lk_Position = lk_BoxDescription["converterOutputFileBoxPosition"].toList();
					moveBoxTo(lk_Box_->outgoingBoxes().toList().first(), QPoint(lk_Position[0].toInt() + 10000, lk_Position[1].toInt() + 10000));
					lk_BoxForId[lk_BoxDescription["converterOutputFileBoxId"].toString()] = lk_Box_->outgoingBoxes().toList().first();
				}
			}
		}
	}
	foreach (QVariant lk_Item, ak_Description["inputFileListBoxes"].toList())
	{
		tk_YamlMap lk_BoxDescription = lk_Item.toMap();
		QString ls_Id = lk_BoxDescription["id"].toString();
		tk_YamlSequence lk_Position = lk_BoxDescription["position"].toList();
		lk_BoxForId[ls_Id] = addInputFileListBox();
		moveBoxTo(lk_BoxForId[ls_Id], QPoint(lk_Position[0].toInt() + 10000, lk_Position[1].toInt() + 10000));
		tk_YamlSequence lk_Paths = lk_BoxDescription["paths"].toList();
		foreach (QVariant ls_Path, lk_Paths)
			dynamic_cast<k_FileListBox*>(lk_BoxForId[ls_Id])->addPath(ls_Path.toString());
	}
	foreach (QVariant lk_Item, ak_Description["connections"].toList())
	{
		tk_YamlSequence lk_Pair = lk_Item.toList();
		if (lk_BoxForId.contains(lk_Pair[0].toString()) && lk_BoxForId.contains(lk_Pair[1].toString()))
			connectBoxes(lk_BoxForId[lk_Pair[0].toString()], lk_BoxForId[lk_Pair[1].toString()]);
	}
	
	redraw();
	setHasUnsavedChanges(false);
	emit showAllRequested();
}


void k_Desktop::setHasUnsavedChanges(bool ab_Flag)
{
	mb_HasUnsavedChanges = ab_Flag;
	mk_PipelineMainWindow.toggleUi();
}


bool k_Desktop::hasUnsavedChanges() const
{
	return mb_HasUnsavedChanges;
}


QSet<IDesktopBox*> k_Desktop::selectedBoxes() const
{
	return mk_SelectedBoxes;
}


bool k_Desktop::useFileTrackerIfAvailable() const
{
	return mb_UseFileTrackerIfAvailable;
}


void k_Desktop::clearAll()
{
	while (!mk_Boxes.empty())
		removeBox(mk_Boxes.toList().first());
}


void k_Desktop::refresh()
{
	foreach (IDesktopBox* lk_Box_, mk_Boxes)
		lk_Box_->toggleUi();
}


void k_Desktop::redraw()
{
	redrawSelection();
	redrawBatchFrame();
	foreach (QGraphicsPathItem* lk_Arrow_, mk_Arrows.keys())
		updateArrow(lk_Arrow_);
}


void k_Desktop::start(bool ab_UseFileTrackingIfAvailable)
{
	mb_UseFileTrackerIfAvailable = ab_UseFileTrackingIfAvailable;
	// collect all script boxes
	mk_RemainingScriptBoxes.clear();
	foreach (IDesktopBox* lk_Box_, mk_Boxes)
	{
		IScriptBox* lk_ScriptBox_ = dynamic_cast<IScriptBox*>(lk_Box_);
		if (lk_ScriptBox_)
			mk_RemainingScriptBoxes.insert(lk_ScriptBox_);
	}
	if (mk_RemainingScriptBoxes.empty())
	{
		mk_Proteomatic.showMessageBox("Error", "There are no script boxes.");
		return;
	}
	
	bool lb_EverythingGravy = true;
	// simulate a run, check whether everything's gravy
	foreach (IScriptBox* lk_Box_, mk_RemainingScriptBoxes)
	{
		QString ls_Error;
		if (!lk_Box_->checkReady(ls_Error))
			lb_EverythingGravy = false;
	}
	if (!lb_EverythingGravy)
	{
		mk_Proteomatic.showMessageBox("Error", "There are script boxes that are not ready.");
		return;
	}
	
	// check whether output files are aleady there
	QList<IScriptBox*> lk_ScriptBoxesWithExistingOutputFiles;
	
	foreach (IScriptBox* lk_Box_, mk_RemainingScriptBoxes)
		if (lk_Box_->hasExistingOutputFiles())
			lk_ScriptBoxesWithExistingOutputFiles.push_back(lk_Box_);
	
	if (!lk_ScriptBoxesWithExistingOutputFiles.empty())
		if (mk_Proteomatic.showMessageBox("Attention", "Some output files already exist. If you continue, only the missing output files will be generated, and scripts with already existing output files will not be run.", ":icons/emblem-important.png", QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Ok, QMessageBox::Cancel) != QMessageBox::Ok)
			return;

	// skip all ->processor<- scripts that have already existing output files,
	// converter script boxes will just silently skip the existing files.
	foreach (IScriptBox* lk_Box_, lk_ScriptBoxesWithExistingOutputFiles)
		if (lk_Box_->script()->type() == r_ScriptType::Processor)
			mk_RemainingScriptBoxes.remove(lk_Box_);
		
	foreach (IScriptBox* lk_Box_, mk_RemainingScriptBoxes)
		mk_RemainingScriptBoxIterationKeys[lk_Box_] = lk_Box_->iterationKeys();

	// pick a box, start it
	IScriptBox* lk_Box_ = pickNextScriptBox();
	if (lk_Box_)
	{
		QString ls_IterationKey = mk_RemainingScriptBoxIterationKeys[lk_Box_].takeFirst();
		lk_Box_->start(ls_IterationKey);
		emit pipelineIdle(false);
	}
}


void k_Desktop::abort()
{
	// abort current script
	IScriptBox* lk_ScriptBox_ = dynamic_cast<IScriptBox*>(mk_CurrentScriptBox_);
	if (lk_ScriptBox_)
		lk_ScriptBox_->abort();
}


void k_Desktop::showAll()
{
	if (mk_Boxes.empty())
		return;

	// reset scaling to 1.0
	md_Scale = 1.0;
	QMatrix lk_Matrix = this->matrix();
	lk_Matrix.setMatrix(md_Scale, lk_Matrix.m12(), lk_Matrix.m21(), md_Scale, lk_Matrix.dx(), lk_Matrix.dy());
	this->setMatrix(lk_Matrix);
	
	// determine bounding box
	QRectF lk_Rect;
	foreach (IDesktopBox* lk_Box_, mk_Boxes)
		lk_Rect = lk_Rect.united(lk_Box_->rect());
	lk_Rect.adjust(-10.0, -10.0, 10.0, 10.0);
	
	// center
	centerOn(lk_Rect.center());
	
	// adjust scaling
	QPointF lk_A = mapToScene(QPoint(0, 0));
	QPointF lk_B = mapToScene(QPoint(frameRect().width(), frameRect().height()));
	double ld_WindowX = lk_B.x() - lk_A.x();
	double ld_WindowY = lk_B.y() - lk_A.y();
	double ld_SceneX = lk_Rect.width();
	double ld_SceneY = lk_Rect.height();
	double a = ld_WindowX / ld_SceneX;
	if (ld_WindowY / ld_SceneY < a)
		a = ld_WindowY / ld_SceneY;
	if (a < 1.0)
	{
		md_Scale *= a;
		md_Scale = std::max<double>(md_Scale, 0.3);
		md_Scale = std::min<double>(md_Scale, 1.0);
		QMatrix lk_Matrix = this->matrix();
		lk_Matrix.setMatrix(md_Scale, lk_Matrix.m12(), lk_Matrix.m21(), md_Scale, lk_Matrix.dx(), lk_Matrix.dy());
		this->setMatrix(lk_Matrix);
	}
}


void k_Desktop::clearPrefixForAllScripts()
{
	foreach (IDesktopBox* lk_Box_, mk_Boxes)
	{
		k_ScriptBox* lk_ScriptBox_ = dynamic_cast<k_ScriptBox*>(lk_Box_);
		if (lk_ScriptBox_)
			lk_ScriptBox_->clearPrefixButtonClicked();
	}
}

void k_Desktop::proposePrefixForAllScripts()
{
	QSet<IScriptBox*> lk_BoxSet;
	foreach (IDesktopBox* lk_Box_, mk_Boxes)
	{
		IScriptBox* lk_ScriptBox_ = dynamic_cast<IScriptBox*>(lk_Box_);
		if (lk_ScriptBox_)
			lk_BoxSet.insert(lk_ScriptBox_);
	}
	while (true)
	{
		IScriptBox* lk_ScriptBox_ = NULL;
		foreach (IScriptBox* lk_SetBox_, lk_BoxSet)
		{
			bool lb_Good = false;
			IDesktopBox* lk_DesktopBox_ = dynamic_cast<IDesktopBox*>(lk_SetBox_);
			if (lk_DesktopBox_)
				lb_Good = (incomingScriptBoxes(lk_DesktopBox_).intersect(lk_BoxSet).size() == 0);
			if (lb_Good)
			{
				lk_ScriptBox_ = lk_SetBox_;
				break;
			}
		}
		
		if (!lk_ScriptBox_)
			break;
		
		lk_BoxSet.remove(lk_ScriptBox_);
		k_ScriptBox* lk_KScriptBox_ = dynamic_cast<k_ScriptBox*>(lk_ScriptBox_);
		if (lk_KScriptBox_ && lk_ScriptBox_->script()->type() != r_ScriptType::Converter)
			lk_KScriptBox_->proposePrefixButtonClicked(false);
	}
}


void k_Desktop::boxMovedOrResized(QPoint ak_Delta)
{
	IDesktopBox* lk_Sender_ = dynamic_cast<IDesktopBox*>(sender());
	if (lk_Sender_ && mk_ArrowsForBox.contains(lk_Sender_))
		foreach (QGraphicsPathItem* lk_Arrow_, mk_ArrowsForBox[lk_Sender_])
			updateArrow(lk_Arrow_);
	foreach (IDesktopBox* lk_Box_, mk_SelectedBoxes)
		foreach (QGraphicsPathItem* lk_Arrow_, mk_ArrowsForBox[lk_Box_])
			updateArrow(lk_Arrow_);
	redraw();
}


void k_Desktop::boxClicked(QMouseEvent* event)
{
	Qt::KeyboardModifiers le_Modifiers = event->modifiers();
	IDesktopBox* lk_Box_ = dynamic_cast<IDesktopBox*>(sender());
	if (md_BoxZ > 1000.0)
	{
		md_BoxZ = 0.0;
		foreach (IDesktopBox* lk_Box_, mk_Boxes)
			mk_ProxyWidgetForBox[lk_Box_]->setZValue(md_BoxZ);
	}
	md_BoxZ += 0.01;
	mk_ProxyWidgetForBox[lk_Box_]->setZValue(md_BoxZ);
	// if z values get too high, reset all boxes z values
	if (!lk_Box_)
		return;
	if (dynamic_cast<IScriptBox*>(lk_Box_))
	{
		if (mk_CurrentScriptBox_ != lk_Box_)
			mb_Error = false;
		setCurrentScriptBox(dynamic_cast<IScriptBox*>(lk_Box_));
	}
	
	if ((le_Modifiers & Qt::ControlModifier) == Qt::ControlModifier)
	{
		if (mk_SelectedBoxes.contains(lk_Box_))
			mk_SelectedBoxes.remove(lk_Box_);
		else
			mk_SelectedBoxes.insert(lk_Box_);
	}
	else
	{
		if (!mk_SelectedBoxes.contains(lk_Box_))
			clearSelection();
		mk_SelectedBoxes.insert(lk_Box_);
	}
	redrawSelection();
	moveSelectedBoxesStart(lk_Box_);
	emit selectionChanged();
}


void k_Desktop::arrowPressed()
{
	if (mb_Running)
		return;
	
	mk_ArrowStartBox_ = dynamic_cast<IDesktopBox*>(sender());
	mk_ArrowEndBox_ = NULL;
	mk_UserArrowPathItem_ = mk_GraphicsScene.
		addPath(QPainterPath(), QPen(QColor(TANGO_ALUMINIUM_3)), QBrush(QColor(TANGO_ALUMINIUM_3)));
	mk_UserArrowPathItem_->setZValue(1000.0);
}


void k_Desktop::arrowReleased()
{
	if (mk_ArrowStartBox_ && mk_ArrowEndBox_)
		connectBoxes(mk_ArrowStartBox_, mk_ArrowEndBox_);
	
	if (mk_ArrowStartBox_ && !mk_ArrowEndBox_)
	{
		// show scripts menu and insert a script box here
		if (!boxAt(mk_ArrowEndPoint))
		{
			mk_ArrowStartBoxAutoConnect_ = mk_ArrowStartBox_;
			QAction* lk_Action_ = mk_Proteomatic.proteomaticScriptsMenu()->exec(mapFromScene(mk_ArrowEndPoint) + mapToGlobal(pos()) - QPoint(8, 8));
			if (!lk_Action_)
				mk_ArrowStartBoxAutoConnect_ = NULL;
		}
	}
	
	mk_ArrowStartBox_ = NULL;
	mk_ArrowEndBox_ = NULL;
	if (mk_UserArrowPathItem_)
	{
		delete mk_UserArrowPathItem_;
		mk_UserArrowPathItem_ = NULL;
	}
}


void k_Desktop::boxBatchModeChanged(bool ab_Enabled)
{
	IDesktopBox* lk_Box_ = dynamic_cast<IDesktopBox*>(sender());
	if (!lk_Box_)
		return;
	
	if (ab_Enabled)
		mk_BatchBoxes.insert(lk_Box_);
	else
		mk_BatchBoxes.remove(lk_Box_);
	
	redrawBatchFrame();
	foreach (QGraphicsPathItem* lk_Arrow_, mk_ArrowsForBox[lk_Box_])
		updateArrow(lk_Arrow_);
	mb_HasUnsavedChanges = true;
	mk_PipelineMainWindow.toggleUi();
}


void k_Desktop::updateArrow(QGraphicsPathItem* ak_Arrow_)
{
	IDesktopBox* lk_ArrowStartBox_ = mk_Arrows[ak_Arrow_].first;
	IDesktopBox* lk_ArrowEndBox_ = mk_Arrows[ak_Arrow_].second;
	
	QPointF lk_Start = boxLocation(lk_ArrowStartBox_);
	QPointF lk_End = boxLocation(lk_ArrowEndBox_);
	
	/*
	QPainterPath lk_Path;
	lk_Path.moveTo(lk_Start);
	lk_Path.lineTo(lk_End);

	QPainterPath lk_Outline;
	lk_Outline = grownPathForBox(lk_ArrowStartBox_, 0 + (mk_SelectedBoxes.contains(lk_ArrowStartBox_) ? 3 : 0) + (lk_ArrowStartBox_->batchMode() ? 4 : 0));
	lk_Path = lk_Path.subtracted(lk_Outline);
	lk_Outline = grownPathForBox(lk_ArrowEndBox_, 0 + (mk_SelectedBoxes.contains(lk_ArrowEndBox_) ? 3 : 0) + (lk_ArrowEndBox_->batchMode() ? 4 : 0));
	lk_Path = lk_Path.subtracted(lk_Outline);

	lk_Start = lk_Path.pointAtPercent(0.0);
	lk_End = lk_Path.pointAtPercent(1.0);
	*/
	

	lk_Start = intersectLineWithBox(lk_End, lk_Start, lk_ArrowStartBox_);
	lk_End = intersectLineWithBox(lk_Start, lk_End, lk_ArrowEndBox_);
	
	QPen lk_Pen(TANGO_ALUMINIUM_3);
	QBrush lk_Brush(TANGO_ALUMINIUM_3);
	if ((dynamic_cast<IFileBox*>(lk_ArrowStartBox_)) && lk_ArrowStartBox_->batchMode())
	{
		lk_Pen = QPen(TANGO_BUTTER_2);
		lk_Brush = QBrush(TANGO_BUTTER_2);
	}
	ak_Arrow_->setPen(lk_Pen);
	ak_Arrow_->setBrush(lk_Brush);
	
	updateArrowInternal(ak_Arrow_, lk_Start, lk_End);
	mk_ArrowProxy[ak_Arrow_]->setLine(QLineF(QPointF(lk_Start), QPointF(lk_End)));
}


void k_Desktop::redrawSelection(bool ab_DontCallOthers)
{
	QPainterPath lk_Path;
	
	foreach (IDesktopBox* lk_Box_, mk_SelectedBoxes)
	{
		int li_Grow = 3;
		if (lk_Box_ == mk_CurrentScriptBox_)
			li_Grow += 3;
		if (lk_Box_->batchMode())
			li_Grow += 4;
		lk_Path = lk_Path.united(grownPathForBox(lk_Box_, li_Grow));
	}
	
	foreach (QGraphicsPathItem* lk_Arrow_, mk_SelectedArrows)
		lk_Path = lk_Path.united(grownPathForArrow(lk_Arrow_, 3));
	
	mk_SelectionGraphicsPathItem_->setPath(lk_Path);
	
	lk_Path = QPainterPath();
	
	if (mk_CurrentScriptBox_)
	{
		QColor lk_FrameColor;
		QColor lk_BackgroundColor;
		if (mb_Running)
		{
			lk_FrameColor = QColor(TANGO_CHAMELEON_2);
			lk_BackgroundColor = QColor(TANGO_CHAMELEON_0);
		}
		else
		{
			if (mb_Error)
			{
				lk_FrameColor = QColor(TANGO_SCARLET_RED_2);
				lk_BackgroundColor = QColor(TANGO_SCARLET_RED_0);
			}
			else
			{
				lk_FrameColor = QColor(TANGO_SKY_BLUE_2);
				lk_BackgroundColor = QColor(TANGO_SKY_BLUE_0);
			}
		}
		QPen lk_Pen = mk_CurrentScriptBoxGraphicsPathItem_->pen();
		lk_Pen.setColor(lk_FrameColor);
		mk_CurrentScriptBoxGraphicsPathItem_->setPen(lk_Pen);
		
		QBrush lk_Brush = mk_CurrentScriptBoxGraphicsPathItem_->brush();
		lk_BackgroundColor.setHsvF(lk_BackgroundColor.hueF(), lk_BackgroundColor.saturationF() * 0.3, lk_BackgroundColor.valueF() * 1.0);
		lk_Brush.setColor(lk_BackgroundColor);
		lk_Brush.setStyle(Qt::SolidPattern);
		mk_CurrentScriptBoxGraphicsPathItem_->setBrush(lk_Brush);
		
		lk_Path = grownPathForBox(mk_CurrentScriptBox_, 3);
	}
	
	mk_CurrentScriptBoxGraphicsPathItem_->setPath(lk_Path);
	
	if (!ab_DontCallOthers)
		redrawBatchFrame(true);
}


void k_Desktop::deleteSelected()
{
	QList<IDesktopBox*> lk_BoxDeleteList = mk_SelectedBoxes.toList();
	QList<QGraphicsPathItem*> lk_ArrowDeleteList = mk_SelectedArrows.toList();

	foreach (QGraphicsPathItem* lk_Arrow_, lk_ArrowDeleteList)
	{
		// skip this arrow if it points to a delete-protected box or it comes from a delete-protected box
		if ((mk_Arrows[lk_Arrow_].first->protectedFromUserDeletion() && dynamic_cast<k_InputGroupProxyBox*>(mk_Arrows[lk_Arrow_].first) != NULL) || 
			(mk_Arrows[lk_Arrow_].second->protectedFromUserDeletion() && dynamic_cast<IScriptBox*>(mk_Arrows[lk_Arrow_].first) != NULL))
			mk_SelectedArrows.remove(lk_Arrow_);
		else
			disconnectBoxes(mk_Arrows[lk_Arrow_].first, mk_Arrows[lk_Arrow_].second);
	}
	
	while (!mk_SelectedBoxes.empty())
	{
		IDesktopBox* lk_First_ = mk_SelectedBoxes.toList().first();
		// skip this box if it's protected from deletion
		if (lk_First_->protectedFromUserDeletion())
			mk_SelectedBoxes.remove(lk_First_);
		else
			removeBox(lk_First_);
	}
	
	redrawSelection();
	redrawBatchFrame();
	emit selectionChanged();
}


void k_Desktop::redrawBatchFrame(bool ab_DontCallOthers)
{
	QPainterPath lk_Path;
	foreach (IDesktopBox* lk_Box_, mk_BatchBoxes)
	{
		int li_Grow = 3;
		if (lk_Box_ == mk_CurrentScriptBox_)
			li_Grow += 4;
		lk_Path = lk_Path.united(grownPathForBox(lk_Box_, li_Grow));
		IFileBox* lk_FileBox_ = dynamic_cast<IFileBox*>(lk_Box_);
		if (lk_FileBox_)
		{
			foreach (IDesktopBox* lk_PeerBox_, lk_Box_->outgoingBoxes())
			{
				if (!dynamic_cast<IScriptBox*>(lk_PeerBox_))
					continue;
				QPointF p0 = boxLocation(lk_Box_);
				QPointF p1 = boxLocation(lk_PeerBox_);
				QPointF p0c = p0;
				QPointF p1c = p1;
				intersectLineWithBox(p0c, p1c, lk_Box_);
				intersectLineWithBox(p0c, p1c, lk_PeerBox_);
				QPointF m0 = p0c + (p1c - p0c) * 0.1;
				QPointF m1 = p0c + (p1c - p0c) * 0.9;
				
				QSize s0 = dynamic_cast<k_DesktopBox*>(lk_Box_)->size();
				QSize s1 = dynamic_cast<k_DesktopBox*>(lk_PeerBox_)->size();
				double r0 = s0.width();
				r0 = std::min<double>(r0, s0.height());
				double r1 = s1.width();
				r1 = std::min<double>(r1, s1.height());
				
				r0 *= 0.5;
				r1 *= 0.5;
				
				QPointF lk_Dir = p1 - p0;
				double lf_Length = lk_Dir.x() * lk_Dir.x() + lk_Dir.y() * lk_Dir.y();
				if (lf_Length > 1.0)
				{
					lf_Length = sqrt(lf_Length);
					QPointF lk_Normal = QPointF(-lk_Dir.y(), lk_Dir.x()) / lf_Length;
					QPainterPath lk_SubPath;
					lk_SubPath.moveTo(p0 + lk_Normal * r0);
					lk_SubPath.cubicTo(m0, m1, p1 + lk_Normal * r1);
					lk_SubPath.lineTo(p1 - lk_Normal * r1);
					lk_SubPath.cubicTo(m1, m0, p0 - lk_Normal * r0);
					lk_SubPath.lineTo(p0 + lk_Normal * r0);
					lk_Path = lk_Path.united(lk_SubPath);
				}
			}
		}
	}
	mk_BatchGraphicsPathItem_->setPath(lk_Path);
	if (!ab_DontCallOthers)
		redrawSelection(true);
}


void k_Desktop::clearSelection()
{
	mk_SelectedBoxes.clear();
	mk_SelectedArrows.clear();
	emit selectionChanged();
}


void k_Desktop::scriptStarted()
{
	IScriptBox* lk_ScriptBox_ = dynamic_cast<IScriptBox*>(sender());
	setCurrentScriptBoxForce(lk_ScriptBox_);
	lk_ScriptBox_->showOutputBox(true);
	if (mk_RemainingScriptBoxIterationKeys[lk_ScriptBox_].empty())
	{
		mk_RemainingScriptBoxes.remove(lk_ScriptBox_);
		mk_RemainingScriptBoxIterationKeys.remove(lk_ScriptBox_);
	}
	mb_Running = true;
	mb_Error = false;
	redrawSelection();
	mk_PipelineMainWindow.toggleUi();
}


void k_Desktop::scriptFinished(int ai_ExitCode)
{
	IScriptBox* lk_ScriptBox_ = dynamic_cast<IScriptBox*>(sender());
	if (ai_ExitCode == 0)
	{
		// do next iteration if there are still some left from sender
		IScriptBox* lk_NextBox_ = lk_ScriptBox_;
		// if there are no iterations left, pick next script box
		if ((!mk_RemainingScriptBoxIterationKeys.contains(lk_ScriptBox_)) || (mk_RemainingScriptBoxIterationKeys[lk_ScriptBox_].empty()))
			lk_NextBox_ = pickNextScriptBox();
		if (lk_NextBox_)
		{
			QString ls_IterationKey = mk_RemainingScriptBoxIterationKeys[lk_NextBox_].takeFirst();
			lk_NextBox_->start(ls_IterationKey);
		}
		else
		{
			mb_Running = false;
			emit pipelineIdle(true);
		}
	}
	else
	{
		mb_Running = false;
		lk_ScriptBox_->showOutputBox(true);
		mb_Error = true;
		emit pipelineIdle(true);
	}
	redrawSelection();
	mk_PipelineMainWindow.toggleUi();
	refresh();
}


void k_Desktop::setCurrentScriptBox(IScriptBox* ak_ScriptBox_)
{
	if (mb_Running)
		return;
	setCurrentScriptBoxForce(ak_ScriptBox_);
}


void k_Desktop::setCurrentScriptBoxForce(IScriptBox* ak_ScriptBox_)
{
	mk_CurrentScriptBox_ = dynamic_cast<IDesktopBox*>(ak_ScriptBox_);
	mk_PipelineMainWindow.setCurrentScriptBox(ak_ScriptBox_);
}


void k_Desktop::keyPressEvent(QKeyEvent* event)
{
	QGraphicsView::keyPressEvent(event);
	if (!event->isAccepted())
	{
		if ((event->matches(QKeySequence::Delete) || (event->key() == Qt::Key_Backspace)) && !mb_Running)
			deleteSelected();
	}
}


void k_Desktop::mousePressEvent(QMouseEvent* event)
{
	QPointF lk_Position = this->mapToScene(event->pos());
	if (mk_GraphicsScene.items(lk_Position).empty())
	{
		if ((event->modifiers() & Qt::ControlModifier) == 0)
		{
			clearSelection();
/*			mk_CurrentScriptBox_ = NULL;
			mk_PipelineMainWindow.setPaneLayoutWidget(NULL);*/
			redrawSelection();
		}
	}
	else
	{
		QList<QGraphicsItem*> lk_ItemList = mk_GraphicsScene.items(lk_Position);
		if (!lk_ItemList.empty())
		{
			QGraphicsLineItem* lk_ProxyLine_ = NULL;
			for (int i = lk_ItemList.size() - 1; i >= 0; --i)
			{
				lk_ProxyLine_ = dynamic_cast<QGraphicsLineItem*>(lk_ItemList[i]);
				if (lk_ProxyLine_)
					break;
			}
			if (lk_ProxyLine_)
			{
				if ((event->modifiers() & Qt::ControlModifier) == 0)
				{
					clearSelection();
					redrawSelection();
				}
				QGraphicsPathItem* lk_Arrow_ = mk_ArrowForProxy[lk_ProxyLine_];
				if (mk_SelectedArrows.contains(lk_Arrow_))
					mk_SelectedArrows.remove(lk_Arrow_);
				else
					mk_SelectedArrows.insert(lk_Arrow_);
				redrawSelection();
				emit selectionChanged();
			}
		}
	}
	event->ignore();
	mk_MoveStartPoint = mapToScene(event->globalPos());
	QGraphicsView::mousePressEvent(event);
}


void k_Desktop::mouseReleaseEvent(QMouseEvent* event)
{
	mb_Moving = false;
	event->accept();
	QGraphicsView::mouseReleaseEvent(event);
}


void k_Desktop::mouseMoveEvent(QMouseEvent* event)
{
	QGraphicsView::mouseMoveEvent(event);
	if (mk_ArrowStartBox_)
	{
		mk_ArrowEndBox_ = boxAt(mapToScene(event->pos()));
		mk_ArrowEndBox_ = connectionAllowed(mk_ArrowStartBox_, mk_ArrowEndBox_);
		updateUserArrow(mapToScene(event->pos()));
	}
	if (mb_Moving)
	{
		if (event->buttons() == Qt::NoButton)
			mb_Moving = false;
		else
			moveSelectedBoxes(mapToScene(event->globalPos()) - mk_MoveStartPoint);
	}
}


void k_Desktop::wheelEvent(QWheelEvent* event)
{
	if ((event->modifiers() & Qt::ControlModifier) != 0)
	{
		double ld_ScaleDelta = pow(1.1, fabs(event->delta() / 100.0));
		if (event->delta() < 0)
			ld_ScaleDelta = 1.0 / ld_ScaleDelta;
		md_Scale *= ld_ScaleDelta;
		md_Scale = std::max<double>(md_Scale, 0.3);
		md_Scale = std::min<double>(md_Scale, 1.0);
		QMatrix lk_Matrix = this->matrix();
		lk_Matrix.setMatrix(md_Scale, lk_Matrix.m12(), lk_Matrix.m21(), md_Scale, lk_Matrix.dx(), lk_Matrix.dy());
		this->setMatrix(lk_Matrix);
	}
}


void k_Desktop::updateUserArrow(QPointF ak_MousePosition)
{
	if (!(mk_UserArrowPathItem_ && mk_ArrowStartBox_))
		return;
	
	QPointF lk_Start = boxLocation(mk_ArrowStartBox_);
	QPointF lk_End;
	if (mk_ArrowEndBox_)
		lk_End = boxLocation(mk_ArrowEndBox_);
	else
		lk_End = ak_MousePosition;
	
	lk_Start = intersectLineWithBox(lk_End, lk_Start, mk_ArrowStartBox_);
	if (mk_ArrowEndBox_)
		lk_End = intersectLineWithBox(lk_Start, lk_End, mk_ArrowEndBox_);
	
	updateArrowInternal(mk_UserArrowPathItem_, lk_Start, lk_End);
	mk_ArrowEndPoint = lk_End;
	mk_ArrowDirection = lk_End - lk_Start;
}


IDesktopBox* k_Desktop::connectionAllowed(IDesktopBox* ak_StartBox_, IDesktopBox* ak_EndBox_)
{
	if (!ak_EndBox_)
		return NULL;
	
	// don't connect if the end box is not an input proxy or script box
	// and don't connect if the end box is the start box
	if (((dynamic_cast<IScriptBox*>(ak_EndBox_) == NULL) && (dynamic_cast<k_InputGroupProxyBox*>(ak_EndBox_) == NULL)) || ak_EndBox_ == ak_StartBox_)
		return NULL;
	
	// make sure that the output of one script is not connected to one of its input proxy boxes
	foreach (IDesktopBox* lk_Box_, ak_StartBox_->incomingBoxes())
	{
		foreach (IDesktopBox* lk_PreviousBox_, lk_Box_->incomingBoxes())
		{
			k_InputGroupProxyBox* lk_ProxyBox_ = dynamic_cast<k_InputGroupProxyBox*>(lk_PreviousBox_);
			if (lk_ProxyBox_ && lk_ProxyBox_ == ak_EndBox_)
			{
				return NULL;
			}
		}
	}
	// make sure the arrow end box is not a script box which has ambiguous input groups
	// in that case, the files should go to one of the proxy boxes
	IScriptBox* lk_ScriptBox_ = dynamic_cast<IScriptBox*>(ak_EndBox_);
	if (lk_ScriptBox_)
	{
		if (!lk_ScriptBox_->script()->ambiguousInputGroups().empty())
			return NULL;
	}
	
	if (ak_StartBox_->incomingBoxes().contains(ak_EndBox_))
		return NULL;
	if (ak_StartBox_->outgoingBoxes().contains(ak_EndBox_))
		return NULL;
	
	return ak_EndBox_;
}


QPoint k_Desktop::boxLocation(IDesktopBox* ak_Box_) const
{
	k_DesktopBox* lk_Box_ = dynamic_cast<k_DesktopBox*>(ak_Box_);
	return lk_Box_->pos() + QPoint(lk_Box_->width(), lk_Box_->height()) / 2;
}


void k_Desktop::moveBoxTo(IDesktopBox* ak_Box_, QPoint ak_Position)
{
	k_DesktopBox* lk_Box_ = dynamic_cast<k_DesktopBox*>(ak_Box_);
	lk_Box_->move(ak_Position - QPoint(lk_Box_->width(), lk_Box_->height()) / 2);
}


IDesktopBox* k_Desktop::boxAt(QPointF ak_Point) const
{
	QGraphicsItem* lk_GraphicsItem_ = mk_GraphicsScene.itemAt(ak_Point);
	QGraphicsWidget* lk_GraphicsWidget_ = dynamic_cast<QGraphicsWidget*>(lk_GraphicsItem_);
	if (lk_GraphicsWidget_ == NULL)
		return NULL;
	QGraphicsProxyWidget* lk_GraphicsProxyWidget_ = dynamic_cast<QGraphicsProxyWidget*>(lk_GraphicsWidget_);
	if (lk_GraphicsProxyWidget_ == NULL)
		return NULL;
	return dynamic_cast<IDesktopBox*>(lk_GraphicsProxyWidget_->widget());
}


double k_Desktop::intersect(QPointF p0, QPointF d0, QPointF p1, QPointF d1)
{
	double d = d0.x() * d1.y() - d0.y() * d1.x();
	if (fabs(d) < 0.000001)
		return 0.0;
	return (d1.x() * (p0.y() - p1.y()) + d1.y() * (p1.x() - p0.x())) / d;
}


void k_Desktop::boxConnector(IDesktopBox* ak_Box0_, IDesktopBox* ak_Box1_, QPointF& ak_Point0, QPointF& ak_Point1)
{
	QPointF lk_Point0 = boxLocation(ak_Box0_);
	QPointF lk_Point1 = boxLocation(ak_Box1_);
	ak_Point0 = intersectLineWithBox(lk_Point1, lk_Point0, ak_Box0_);
	ak_Point1 = intersectLineWithBox(lk_Point0, lk_Point1, ak_Box1_);
}


QPointF k_Desktop::intersectLineWithBox(const QPointF& ak_Point0, const QPointF& ak_Point1, IDesktopBox* ak_Box_)
{
	QPointF lk_Dir = ak_Point1 - ak_Point0;
	QPointF lk_Quadrant = QPointF(fabs(lk_Dir.x()), fabs(lk_Dir.y()));
	k_DesktopBox* lk_Box_ = dynamic_cast<k_DesktopBox*>(ak_Box_);
	QPointF lk_Diagonal = QPointF(lk_Box_->width(), lk_Box_->height());
	double t = 1.0;
	if (lk_Quadrant.x() * lk_Diagonal.y() > lk_Quadrant.y() * lk_Diagonal.x())
	{
		QPointF lk_Quadrant = ak_Point0 - ak_Point1;
		if (lk_Quadrant.x() < 0)
			t = intersect(ak_Point0, lk_Dir, boxLocation(ak_Box_) - QPointF(lk_Box_->width(), lk_Box_->height()) * 0.5, QPointF(0, lk_Box_->height()));
		else
			t = intersect(ak_Point0, lk_Dir, boxLocation(ak_Box_) - QPointF(lk_Box_->width(), lk_Box_->height()) * 0.5 + QPointF(lk_Box_->width(), 0), QPointF(0, lk_Box_->height()));
	}
	else
	{
		QPointF lk_Quadrant = ak_Point0 - ak_Point1;
		if (lk_Quadrant.y() < 0)
			t = intersect(ak_Point0, lk_Dir, boxLocation(ak_Box_) - QPointF(lk_Box_->width(), lk_Box_->height()) * 0.5, QPointF(lk_Box_->width(), 0));
		else
			t = intersect(ak_Point0, lk_Dir, boxLocation(ak_Box_) - QPointF(lk_Box_->width(), lk_Box_->height()) * 0.5 + QPointF(0, lk_Box_->height()), QPointF(lk_Box_->width(), 0));
	}
	
	return ak_Point0 + lk_Dir * t;
}


void k_Desktop::updateArrowInternal(QGraphicsPathItem* ak_Arrow_, QPointF ak_Start, QPointF ak_End)
{
	QPainterPath lk_Path;
	
	if (mk_ArrowStartBox_ && dynamic_cast<k_DesktopBox*>(mk_ArrowStartBox_)->frameGeometry().contains(ak_End.toPoint()))
	{
		ak_Arrow_->setPath(lk_Path);
		return;
	}
	
	lk_Path.moveTo(ak_Start);
	lk_Path.lineTo(ak_End);
	
	QPointF lk_Dir = ak_End - ak_Start;
	double ld_Length = sqrt(lk_Dir.x() * lk_Dir.x() + lk_Dir.y() * lk_Dir.y());
	if (ld_Length > 1.0)
	{
		lk_Dir /= ld_Length;
		QPointF lk_Normal = QPointF(-lk_Dir.y(), lk_Dir.x());
		QPolygonF lk_Arrow;
		lk_Arrow << ak_End;
		lk_Arrow << ak_End - lk_Dir * 10.0 + lk_Normal * 3.5;
		lk_Arrow << ak_End - lk_Dir * 7.0;
		lk_Arrow << ak_End - lk_Dir * 10.0 - lk_Normal * 3.5;
		lk_Arrow << ak_End;
		lk_Path.addPolygon(lk_Arrow);
	}

	ak_Arrow_->setPath(lk_Path);
}


QPainterPath k_Desktop::grownPathForBox(IDesktopBox* ak_Box_, int ai_Grow)
{
	QWidget* lk_Widget_ = dynamic_cast<QWidget*>(ak_Box_);
	lk_Widget_->ensurePolished();
	QPainterPath lk_Path;
	lk_Path.addRoundedRect(QRectF(
		lk_Widget_->x() - ai_Grow, lk_Widget_->y() - ai_Grow, 
		lk_Widget_->width() + ai_Grow * 2, lk_Widget_->height() + ai_Grow * 2), 
		8.0 + ai_Grow, 8.0 + ai_Grow);
	return lk_Path;
}

QPainterPath k_Desktop::grownPathForArrow(QGraphicsPathItem* ak_Arrow_, int ai_Grow)
{
	if (!mk_Arrows.contains(ak_Arrow_))
		return QPainterPath();
	
	QPainterPath lk_Path;
	IDesktopBox* lk_StartBox_ = mk_Arrows[ak_Arrow_].first;
	IDesktopBox* lk_EndBox_ = mk_Arrows[ak_Arrow_].second;
	
	QPointF p0 = boxLocation(lk_StartBox_);
	QPointF p1 = boxLocation(lk_EndBox_);
	
	QPointF lk_Dir = p1 - p0;
	double lf_Length = lk_Dir.x() * lk_Dir.x() + lk_Dir.y() * lk_Dir.y();
	if (lf_Length > 1.0)
	{
		lf_Length = sqrt(lf_Length);
		QPointF lk_Normal = QPointF(-lk_Dir.y(), lk_Dir.x()) / lf_Length;
		lk_Path.moveTo(p0 + lk_Normal * ai_Grow);
		lk_Path.lineTo(p1 + lk_Normal * ai_Grow);
		lk_Path.lineTo(p1 - lk_Normal * ai_Grow);
		lk_Path.lineTo(p0 - lk_Normal * ai_Grow);
		lk_Path.lineTo(p0 + lk_Normal * ai_Grow);
	}
	return lk_Path;
}


QPointF k_Desktop::findFreeSpace(QRectF ak_BoundRect, int ai_BoxCount, IDesktopBox* ak_Box_)
{
	QPointF lk_HalfSize = 
		QPointF((double)(dynamic_cast<k_DesktopBox*>(ak_Box_)->width()) * 0.5,
				 (double)(dynamic_cast<k_DesktopBox*>(ak_Box_)->height()) * 0.5);
				 
	if (ai_BoxCount == 0)
		return QPointF(10000.0, 10000.0) - lk_HalfSize;

	QRectF lk_Rect = ak_BoundRect.adjusted(-8.0, -8.0, 8.0, 8.0);
	double p[4] = {lk_Rect.top(), lk_Rect.right(), lk_Rect.bottom(), lk_Rect.left()};
	int best = 0;
	double bestd = fabs(p[0] - 10000.0);
	for (int i = 1; i < 4; ++i)
	{
		double d = fabs(p[i] - 10000.0);
		if (d < bestd)
		{
			bestd = d;
			best = i;
		}
	}
	if (best == 0)
		return QPointF(10000.0, lk_Rect.top() - lk_HalfSize.y()) - lk_HalfSize;
	if (best == 1)
		return QPointF(lk_Rect.right() + lk_HalfSize.x(), 10000.0) - lk_HalfSize;
	if (best == 2)
		return QPointF(10000.0, lk_Rect.bottom() + lk_HalfSize.y()) - lk_HalfSize;
	if (best == 3)
		return QPointF(lk_Rect.left() - lk_HalfSize.x(), 10000.0) - lk_HalfSize;
	return QPointF() - lk_HalfSize;
}


IScriptBox* k_Desktop::pickNextScriptBox()
{
	foreach (IScriptBox* lk_Box_, mk_RemainingScriptBoxes)
	{
		IScriptBox* lk_ScriptBox_ = dynamic_cast<IScriptBox*>(lk_Box_);
		if (lk_ScriptBox_ && lk_ScriptBox_->checkReadyToGo())
			return lk_ScriptBox_;
	}
	return NULL;
}


// this function checks all incoming boxes of ak_Box_ and reports all 
// script boxes among these. if an incoming box is not a script box,
// the function is recursively called until no incoming boxes are left
QSet<IScriptBox*> k_Desktop::incomingScriptBoxes(IDesktopBox* ak_Box_) const
{
	QSet<IScriptBox*> lk_Result;
	foreach (IDesktopBox* lk_OtherBox_, ak_Box_->incomingBoxes())
	{
		IScriptBox* lk_ScriptBox_ = dynamic_cast<IScriptBox*>(lk_OtherBox_);
		if (lk_ScriptBox_)
			lk_Result.insert(lk_ScriptBox_);
		else
			lk_Result.unite(incomingScriptBoxes(lk_OtherBox_));
	}
	return lk_Result;
}


void k_Desktop::dragEnterEvent(QDragEnterEvent* event)
{
	event->acceptProposedAction();
}


void k_Desktop::dragMoveEvent(QDragMoveEvent* event)
{
	if (!boxAt(mapToScene(event->pos())))
		event->acceptProposedAction();
	else
		QGraphicsView::dragMoveEvent(event);
}


void k_Desktop::dropEvent(QDropEvent* event)
{
	// only accept this if there is no box under the mouse pointer
	if (boxAt(mapToScene(event->pos())))
	{
		QGraphicsView::dropEvent(event);
		return;
	}
	event->accept();
	QStringList lk_Files;
	foreach (QUrl lk_Url, event->mimeData()->urls())
	{
		QString ls_Path = lk_Url.toLocalFile();
		if (!ls_Path.isEmpty())
		{
			if (QFileInfo(ls_Path).isFile())
				lk_Files << ls_Path;
		}
	}
	if (!lk_Files.empty())
	{
		k_FileListBox* lk_FileListBox_ = dynamic_cast<k_FileListBox*>(addInputFileListBox());
		if (lk_FileListBox_)
		{
			lk_FileListBox_->addPaths(lk_Files);
			QSize lk_Size = lk_FileListBox_->size();
			lk_Size.setWidth(lk_Size.width() * 0.5);
			lk_Size.setHeight(lk_Size.width() * 0.1);
			lk_FileListBox_->move(mapToScene(event->pos()).toPoint() - QPoint(lk_Size.width(), lk_Size.height()));
		}
	}
}


Qt::DropActions k_Desktop::supportedDropActions() const
{
	return Qt::ActionMask;
}
