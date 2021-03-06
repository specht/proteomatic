/*
Copyright (c) 2007-2010 Michael Specht

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
#include "ClickableGraphicsProxyWidget.h"
#include "DesktopBoxFactory.h"
#include "DesktopBox.h"
#include "FileListBox.h"
#include "SnippetBox.h"
#include "IFileBox.h"
#include "InputGroupProxyBox.h"
#include "IScriptBox.h"
#include "ScriptBox.h"
#include "SearchMenu.h"
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
    , mk_CurrentScriptBox_(NULL)
    , mb_Running(false)
    , mb_Error(false)
    , md_BoxZ(0.0)
    , mb_HasUnsavedChanges(false)
    , mb_Moving(false)
    , mb_UseFileTrackerIfAvailable(true)
    , mb_Animating(false)
    , mb_LassoSelecting(false)
    , mk_LassoCursor(QCursor(QPixmap(":/icons/lasso-cursor.png"), 10, 22))
    , mb_GlobalUpdateRequested(false)
{
    connect(this, SIGNAL(requestGlobalUpdate()), this, SLOT(globalUpdate()), Qt::QueuedConnection);
    connect(&mk_PipelineMainWindow, SIGNAL(forceRefresh()), this, SLOT(refresh()));
    connect(&mk_FileSystemWatcher, SIGNAL(directoryChanged(const QString&)), this, SLOT(refresh()));
    connect(this, SIGNAL(showAllRequested()), this, SLOT(showAll()));
    connect(&mk_AnimationTimer, SIGNAL(timeout()), this, SLOT(animationTimeout()));
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setAcceptDrops(true);
    setTransformationAnchor(QGraphicsView::AnchorViewCenter);
    setResizeAnchor(QGraphicsView::AnchorViewCenter);
    setScene(&mk_GraphicsScene);
    setRenderHint(QPainter::Antialiasing, true);
    setRenderHint(QPainter::TextAntialiasing, true);
    setRenderHint(QPainter::SmoothPixmapTransform, true);
    setBackgroundBrush(QBrush(QColor("#f8f8f8")));
    setSceneRect(-10000.0, -10000.0, 20000.0, 20000.0);
    setDragMode(QGraphicsView::ScrollHandDrag);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    centerOn(0.5, 0.5);
    
    QPen lk_Pen(QColor(TANGO_ALUMINIUM_3));
    lk_Pen.setWidthF(1.5);
    lk_Pen.setStyle(Qt::DashLine);
    mk_SelectionGraphicsPathItem_ = mk_GraphicsScene.addPath(QPainterPath(), lk_Pen);
    mk_SelectionGraphicsPathItem_->setZValue(-4.0);
    mk_LassoGraphicsPathItem_ = mk_GraphicsScene.addPath(QPainterPath(), lk_Pen);
    mk_LassoGraphicsPathItem_->setZValue(-4.0);
    
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

    
IDesktopBox* k_Desktop::addInputFileListBox(bool ab_AutoAdjust)
{
    IDesktopBox* lk_Box_ = k_DesktopBoxFactory::makeFileListBox(this, mk_Proteomatic);
    k_DesktopBox* lk_DesktopBox_ = dynamic_cast<k_DesktopBox*>(lk_Box_);
    addBox(lk_Box_, true, ab_AutoAdjust, 270, 120);
    k_FileListBox* lk_FileListBox_ = dynamic_cast<k_FileListBox*>(lk_Box_);
    if (lk_FileListBox_)
        connect(lk_FileListBox_, SIGNAL(filenamesChanged()), this, SLOT(updateWatchedDirectories()));
    refresh();
    return lk_DesktopBox_;
}


IDesktopBox* k_Desktop::addSnippetBox(bool ab_AutoAdjust)
{
    IDesktopBox* lk_Box_ = k_DesktopBoxFactory::makeSnippetBox(this, mk_Proteomatic);
    k_DesktopBox* lk_DesktopBox_ = dynamic_cast<k_DesktopBox*>(lk_Box_);
    addBox(lk_Box_, true, ab_AutoAdjust, 350, 200);
    refresh();
    return lk_DesktopBox_;
}


IDesktopBox* k_Desktop::addScriptBox(const QString& as_ScriptUri, bool ab_AutoAdjust, 
                                     bool ab_AutoAddFileBoxIfEmpty)
{
    int li_OldBoxCount = mk_Boxes.size();
    
    bool lb_DesktopWasEmpty = mk_Boxes.empty();
    
    IDesktopBox* lk_Box_ = k_DesktopBoxFactory::makeScriptBox(as_ScriptUri, this, mk_Proteomatic);
    if (lk_Box_)
    {
        IScriptBox* lk_ScriptBox_ = dynamic_cast<IScriptBox*>(lk_Box_);
        connect(dynamic_cast<QObject*>(lk_ScriptBox_), SIGNAL(outputDirectoryChanged()), this, SLOT(updateWatchedDirectories()));
        mk_BoxForScript[lk_ScriptBox_->script()] = lk_ScriptBox_;
        addBox(lk_Box_, false, false);
        connect(dynamic_cast<QObject*>(lk_ScriptBox_), SIGNAL(scriptStarted()), this, SLOT(scriptStarted()));
        connect(dynamic_cast<QObject*>(lk_ScriptBox_), SIGNAL(scriptFinished(int)), this, SLOT(scriptFinished(int)));
        mb_Error = false;
        
        // add input group proxy boxes
        QSet<QString> lk_InputGroups = lk_ScriptBox_->script()->inputGroupKeys().toSet();
        foreach (QString ls_GroupKey, lk_ScriptBox_->script()->ambiguousInputGroups())
        {
            QString ls_GroupLabel = lk_ScriptBox_->script()->inputGroupLabel(ls_GroupKey);
            if (ls_GroupLabel.length() > 0)
                ls_GroupLabel[0] = ls_GroupLabel[0].toUpper();
            IDesktopBox* lk_ProxyBox_ = k_DesktopBoxFactory::makeInputGroupProxyBox(this, mk_Proteomatic, ls_GroupLabel + " files", ls_GroupKey);
            if (lk_ProxyBox_)
            {
                addBox(lk_ProxyBox_, false, false);
                connectBoxes(lk_ProxyBox_, lk_Box_);
            }
            lk_InputGroups.remove(ls_GroupKey);
        }
        // ...and add another input group proxy box for remaining files, if necessary
        if (!lk_ScriptBox_->script()->ambiguousInputGroups().empty() && !lk_InputGroups.empty())
        {
            // add another 'remaining files' input proxy box and disallow direct connections to the script box
            IDesktopBox* lk_ProxyBox_ = k_DesktopBoxFactory::makeInputGroupProxyBox(this, mk_Proteomatic, "Remaining input files", "");
            if (lk_ProxyBox_)
            {
                addBox(lk_ProxyBox_, false, false);
                connectBoxes(lk_ProxyBox_, lk_Box_);
            }
        }
        if (ab_AutoAddFileBoxIfEmpty)
        {
            // if this script has input files, and there is not input file box (and no snippet)
            // yet, add and connect an input file box
            if (lb_DesktopWasEmpty && (!lk_ScriptBox_->script()->inputGroupKeys().isEmpty()) && lk_Box_->incomingBoxes().empty())
            {
                //IDesktopBox* lk_InputFileBox_ = k_DesktopBoxFactory::makeFileListBox(this, mk_Proteomatic);
                IDesktopBox* lk_InputFileBox_ = addInputFileListBox(false);
                if (lk_InputFileBox_)
                    connectBoxes(lk_InputFileBox_, lk_Box_);
            }
        }
        
        // place box construct
        QSet<IDesktopBox*> lk_AllBoxes;
        QSize lk_IncomingSize(0, 0);
        QSize lk_ScriptSize(0, 0);
        QSize lk_OutgoingSize(0, 0);
        lk_ScriptSize = dynamic_cast<k_DesktopBox*>(lk_Box_)->size();
        lk_AllBoxes << lk_Box_;
        foreach (IDesktopBox* lk_Other_, lk_Box_->incomingBoxes())
        {
            k_DesktopBox* lk_OtherDesktopBox_ = dynamic_cast<k_DesktopBox*>(lk_Other_);
            if (lk_IncomingSize.width() > 0)
                lk_IncomingSize += QSize(30, 0);
            lk_IncomingSize += QSize(lk_OtherDesktopBox_->width(), 0);
            if (lk_OtherDesktopBox_->height() > lk_IncomingSize.height())
                lk_IncomingSize.setHeight(lk_OtherDesktopBox_->height());
            lk_AllBoxes << lk_Other_;
        }
        foreach (IDesktopBox* lk_Other_, lk_Box_->outgoingBoxes())
        {
            k_DesktopBox* lk_OtherDesktopBox_ = dynamic_cast<k_DesktopBox*>(lk_Other_);
            if (lk_OutgoingSize.width() > 0)
                lk_OutgoingSize += QSize(30, 0);
            lk_OutgoingSize += QSize(lk_OtherDesktopBox_->width(), 0);
            if (lk_OtherDesktopBox_->height() > lk_OutgoingSize.height())
                lk_OutgoingSize.setHeight(lk_OtherDesktopBox_->height());
            lk_AllBoxes << lk_Other_;
        }
        (dynamic_cast<k_ScriptBox*>(lk_Box_))->move(-QPoint(lk_ScriptSize.width() / 2, lk_ScriptSize.height() / 2));
        QRectF lk_AllBoxesBoundingRect = QRectF(dynamic_cast<k_ScriptBox*>(lk_Box_)->frameGeometry());
        int li_Offset = 0;
        foreach (IDesktopBox* lk_Other_, lk_Box_->incomingBoxes())
        {
            k_DesktopBox* lk_OtherDesktopBox_ = dynamic_cast<k_DesktopBox*>(lk_Other_);
            lk_OtherDesktopBox_->move(QPoint(li_Offset - lk_IncomingSize.width() / 2, -lk_ScriptSize.height() / 2 - lk_IncomingSize.height() - 30));
            lk_AllBoxesBoundingRect |= QRectF(lk_OtherDesktopBox_->frameGeometry());
            li_Offset += lk_OtherDesktopBox_->width() + 30;
        }
        li_Offset = 0;
        foreach (IDesktopBox* lk_Other_, lk_Box_->outgoingBoxes())
        {
            k_DesktopBox* lk_OtherDesktopBox_ = dynamic_cast<k_DesktopBox*>(lk_Other_);
            lk_OtherDesktopBox_->move(QPoint(li_Offset - lk_OutgoingSize.width() / 2, lk_ScriptSize.height() / 2 + 30));
            lk_AllBoxesBoundingRect |= QRectF(lk_OtherDesktopBox_->frameGeometry());
            li_Offset += lk_OtherDesktopBox_->width() + 30;
        }
        QRectF lk_BoundingRect = mk_GraphicsScene.itemsBoundingRect();
        QPointF lk_FreeSpace = findFreeSpace(lk_BoundingRect, li_OldBoxCount, lk_AllBoxesBoundingRect);
        foreach (IDesktopBox* lk_MoveBox_, lk_AllBoxes)
            dynamic_cast<k_DesktopBox*>(lk_MoveBox_)->move(dynamic_cast<k_DesktopBox*>(lk_MoveBox_)->pos() + QPoint((int)lk_FreeSpace.x(), (int)lk_FreeSpace.y()));
        
        if (!lb_DesktopWasEmpty)
        {
            mk_SelectedArrows.clear();
            mk_SelectedBoxes.clear();
            foreach (IDesktopBox* lk_MoveBox_, lk_AllBoxes)
                mk_SelectedBoxes << lk_MoveBox_;
        }
        
        if (mk_ArrowStartBoxAutoConnect_)
        {
            if (connectionAllowed(mk_ArrowStartBoxAutoConnect_, lk_Box_))
                connectBoxes(mk_ArrowStartBoxAutoConnect_, lk_Box_);
            k_DesktopBox* lk_ScriptDesktopBox_ = dynamic_cast<k_DesktopBox*>(lk_ScriptBox_);
            if (lk_ScriptDesktopBox_)
            {
                lk_AllBoxesBoundingRect = QRectF();
                foreach (IDesktopBox* lk_Other_, lk_AllBoxes)
                {
                    k_DesktopBox* lk_OtherDesktopBox_ = dynamic_cast<k_DesktopBox*>(lk_Other_);
                    lk_AllBoxesBoundingRect |= QRectF(lk_OtherDesktopBox_->frameGeometry());
                }

                QSize lk_Size = lk_AllBoxesBoundingRect.toRect().size();
                double ld_Length = sqrt(mk_ArrowDirection.x() * mk_ArrowDirection.x() + mk_ArrowDirection.y() * mk_ArrowDirection.y());
                mk_ArrowDirection /= ld_Length;
                if (fabs(mk_ArrowDirection.x()) > fabs(mk_ArrowDirection.y()))
                {
                    lk_Size.setWidth(lk_Size.width() * (1.0 - mk_ArrowDirection.x()) * 0.5 + 0.5);
                    lk_Size.setHeight(lk_Size.height() * (1.0 - mk_ArrowDirection.x()) * 0.5 + 0.5);
                }
                else
                {
                    lk_Size.setWidth(lk_Size.width() * (1.0 - mk_ArrowDirection.y()) * 0.5 + 0.5);
                    lk_Size.setHeight(lk_Size.height() * (1.0 - mk_ArrowDirection.y()) * 0.5 + 0.5);
                }
                
                foreach (IDesktopBox* lk_Other_, lk_AllBoxes)
                {
                    k_DesktopBox* lk_OtherDesktopBox_ = dynamic_cast<k_DesktopBox*>(lk_Other_);
                    lk_OtherDesktopBox_->move((lk_OtherDesktopBox_->pos() + mk_ArrowEndPoint.toPoint() - QPoint(lk_Size.width(), lk_Size.height()) - lk_AllBoxesBoundingRect.topLeft()).toPoint());
                }
            }
        }
        setCurrentScriptBox(lk_ScriptBox_);
        mk_ArrowStartBoxAutoConnect_ = NULL;
        redrawSelection();
        emit selectionChanged();
        refresh();
        if (mk_Proteomatic.stringToBool(mk_Proteomatic.getConfiguration(CONFIG_FOLLOW_NEW_BOXES).toString()))
        {
            if (ab_AutoAdjust)
            {
                if (lb_DesktopWasEmpty)
                    animateAdjustView(true, QSet<IDesktopBox*>(), false);
                else
                    animateAdjustView(false, mk_SelectedBoxes);
            }
        }
    }
    return lk_Box_;
}


void k_Desktop::addBox(IDesktopBox* ak_Box_, bool ab_PlaceBox, bool ab_AutoAdjust, int ai_Width, int ai_Height)
{
    k_DesktopBox* lk_DesktopBox_ = dynamic_cast<k_DesktopBox*>(ak_Box_);
    
    // connect requestGlobalUpdate() and globalUpdate() via a queued connection
    connect(dynamic_cast<QObject*>(ak_Box_), SIGNAL(requestGlobalUpdate()), this, SLOT(globalUpdate()), Qt::QueuedConnection);
    connect(dynamic_cast<QObject*>(ak_Box_), SIGNAL(requestGlobalUpdate()), this, SLOT(markBoxForUpdate()), Qt::DirectConnection);
    
    QRectF lk_BoundingRect = mk_GraphicsScene.itemsBoundingRect();
    k_ClickableGraphicsProxyWidget* lk_ProxyWidget_ = new k_ClickableGraphicsProxyWidget();
    lk_ProxyWidget_->setWidget(lk_DesktopBox_);
    mk_ProxyWidgetForBox[lk_DesktopBox_] = lk_ProxyWidget_;
    mk_GraphicsScene.addItem(lk_ProxyWidget_);
    connect(lk_ProxyWidget_, SIGNAL(pressed()), this, SLOT(bringBoxToFrontSender()));
    
    IFileBox* lk_FileBox_ = dynamic_cast<IFileBox*>(ak_Box_);
    if (lk_FileBox_)
    {
        connect(dynamic_cast<QObject*>(lk_FileBox_), SIGNAL(arrowPressed()), this, SLOT(arrowPressed()));
        connect(dynamic_cast<QObject*>(lk_FileBox_), SIGNAL(arrowReleased()), this, SLOT(arrowReleased()));
    }
    connect(lk_DesktopBox_, SIGNAL(moved(QPoint)), this, SLOT(boxMovedOrResized(QPoint)));
    connect(lk_DesktopBox_, SIGNAL(resized()), this, SLOT(boxMovedOrResized()));
    connect(lk_DesktopBox_, SIGNAL(clicked(QMouseEvent*)), this, SLOT(boxClicked(QMouseEvent*)));
    mk_Boxes.insert(ak_Box_);
    lk_DesktopBox_->resize(ai_Width, ai_Height);
    if (ab_PlaceBox)
    {
        QPointF lk_FreeSpace = findFreeSpace(lk_BoundingRect, mk_Boxes.size() - 1, QRectF(dynamic_cast<QWidget*>(ak_Box_)->frameGeometry()));
        lk_DesktopBox_->move(QPoint((int)lk_FreeSpace.x(), (int)lk_FreeSpace.y()));
    }
    redraw();
    mb_HasUnsavedChanges = true;
    mk_PipelineMainWindow.toggleUi();
    if (ab_PlaceBox && ab_AutoAdjust)
        animateAdjustView(false, QSet<IDesktopBox*>() << ak_Box_);
}


void k_Desktop::removeBox(IDesktopBox* ak_Box_)
{
    if (!mk_Boxes.contains(ak_Box_))
        return;

    if (mk_DeleteBoxStackSet.contains(ak_Box_))
        return;
    
    mk_DeleteBoxStackSet.insert(ak_Box_);

    IScriptBox* lk_ScriptBox_ = dynamic_cast<IScriptBox*>(ak_Box_);

    disconnect(dynamic_cast<QObject*>(ak_Box_), SIGNAL(requestGlobalUpdate()), this, SLOT(globalUpdate()));
    disconnect(dynamic_cast<QObject*>(ak_Box_), SIGNAL(requestGlobalUpdate()), this, SLOT(markBoxForUpdate()));
    
    // explicitely delete incoming input group proxy boxes if this is a script box
    if (lk_ScriptBox_)
    {
        foreach (IDesktopBox* lk_Box_, ak_Box_->incomingBoxes())
        {
            k_InputGroupProxyBox* lk_ProxyBox_ = dynamic_cast<k_InputGroupProxyBox*>(lk_Box_);
            if (lk_ProxyBox_)
                this->removeBox(lk_ProxyBox_);
        }
    }
    else
    {
        // assume this is a file box
        if (ak_Box_->batchMode())
            ak_Box_->invalidateNext(1);
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
    {
        if (mk_BoxForScript.empty())
            setCurrentScriptBox(NULL);
        else
            setCurrentScriptBox(mk_BoxForScript.values().first());
    }
    
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


bool k_Desktop::hasScriptBoxes()
{
    return !mk_BoxForScript.empty();
}


tk_YamlMap k_Desktop::pipelineDescription()
{
    // collect script boxes
    QSet<tk_BoxPair> lk_UnsavedArrows = mk_Arrows.values().toSet();
    QSet<IDesktopBox*> lk_AlreadySavedFileBoxes;
    
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
            lk_Coordinates.push_back(boxLocation(lk_Box_).x());
            lk_Coordinates.push_back(boxLocation(lk_Box_).y());
            lk_ScriptBoxDescription["position"] = lk_Coordinates;
            tk_YamlSequence lk_Size;
            lk_Size.push_back(lk_Box_->rect().width());
            lk_Size.push_back(lk_Box_->rect().height());
            lk_ScriptBoxDescription["size"] = lk_Size;
            lk_ScriptBoxDescription["shortIterationTags"] = lk_ScriptBox_->useShortIterationTags();
            lk_ScriptBoxDescription["outputPrefix"] = lk_ScriptBox_->boxOutputPrefix();
            lk_ScriptBoxDescription["outputDirectory"] = lk_ScriptBox_->boxOutputDirectory();
            lk_ScriptBoxDescription["expanded"] = lk_ScriptBox_->isExpanded();

            // now add active output boxes
            tk_YamlMap lk_ActiveOutputFileBoxes;
            foreach (QString ls_Key, lk_ScriptBox_->script()->outputFileKeys())
            {
                if (lk_ScriptBox_->outputFileActivated(ls_Key))
                {
                    IDesktopBox* lk_OutputFileBox_ = lk_ScriptBox_->boxForOutputFileKey(ls_Key);
                    lk_AlreadySavedFileBoxes << lk_OutputFileBox_;
                    tk_YamlMap lk_Map;
                    tk_YamlSequence lk_Coordinates;
                    lk_Coordinates.push_back(boxLocation(lk_OutputFileBox_).x());
                    lk_Coordinates.push_back(boxLocation(lk_OutputFileBox_).y());
                    lk_Map["position"] = lk_Coordinates;
                    tk_YamlSequence lk_Size;
                    lk_Size.push_back(lk_OutputFileBox_->rect().width());
                    lk_Size.push_back(lk_OutputFileBox_->rect().height());
                    lk_Map["size"] = lk_Size;
                    lk_Map["batchMode"] = lk_OutputFileBox_->batchMode();
                    lk_Map["id"] = (qint64)lk_OutputFileBox_;
                    lk_ActiveOutputFileBoxes[ls_Key] = lk_Map;
                    lk_UnsavedArrows.remove(tk_BoxPair(lk_Box_, lk_OutputFileBox_));
                }
            }
            lk_ScriptBoxDescription["activeOutputFiles"] = lk_ActiveOutputFileBoxes;
            
            // add converter out box
            if (lk_ScriptBox_->script()->type() == r_ScriptType::Converter)
            {
                IDesktopBox* lk_OtherBox_ = lk_Box_->outgoingBoxes().toList().first();
                lk_AlreadySavedFileBoxes << lk_OtherBox_;
                tk_YamlSequence lk_Position;
                lk_Position.push_back(boxLocation(lk_OtherBox_).x());
                lk_Position.push_back(boxLocation(lk_OtherBox_).y());
                lk_ScriptBoxDescription["converterOutputFileBoxPosition"] = lk_Position;
                tk_YamlSequence lk_Size;
                lk_Size.push_back(lk_OtherBox_->rect().width());
                lk_Size.push_back(lk_OtherBox_->rect().height());
                lk_ScriptBoxDescription["converterOutputFileBoxSize"] = lk_Size;
                lk_ScriptBoxDescription["converterOutputFileBoxBatchMode"] = lk_OtherBox_->batchMode();
                lk_ScriptBoxDescription["converterOutputFileBoxId"] = (qint64)lk_OtherBox_;
            }
            
            // now add input proxy boxes
            tk_YamlMap lk_InputProxyBoxes;
            foreach (IDesktopBox* lk_ProxyDesktopBox_, dynamic_cast<IDesktopBox*>(lk_ScriptBox_)->incomingBoxes())
            {
                k_InputGroupProxyBox* lk_ProxyBox_ = dynamic_cast<k_InputGroupProxyBox*>(lk_ProxyDesktopBox_);
                if (lk_ProxyBox_)
                {
                    QString ls_Key = lk_ProxyBox_->groupKey();
                    tk_YamlMap lk_Map;
                    tk_YamlSequence lk_Coordinates;
                    lk_Coordinates.push_back(boxLocation(lk_ProxyBox_).x());
                    lk_Coordinates.push_back(boxLocation(lk_ProxyBox_).y());
                    lk_Map["position"] = lk_Coordinates;
                    tk_YamlSequence lk_Size;
                    lk_Size.push_back(lk_ProxyBox_->rect().width());
                    lk_Size.push_back(lk_ProxyBox_->rect().height());
                    lk_Map["size"] = lk_Size;
                    lk_Map["id"] = (qint64)lk_ProxyDesktopBox_;
                    lk_InputProxyBoxes[ls_Key] = lk_Map;
                    lk_UnsavedArrows.remove(tk_BoxPair(lk_ProxyBox_, lk_Box_));
                    lk_AlreadySavedFileBoxes << lk_ProxyBox_;
                }
            }
            lk_ScriptBoxDescription["inputProxyBoxes"] = lk_InputProxyBoxes;
            
            lk_ScriptBoxes.push_back(lk_ScriptBoxDescription);
        }
    }
    lk_Description["scriptBoxes"] = lk_ScriptBoxes;

    // now come the input file boxes
    tk_YamlSequence lk_InputFileListBoxes;
    foreach (IDesktopBox* lk_Box_, mk_Boxes)
    {
        // is it a true input file box?
        k_FileListBox* lk_FileListBox_ = dynamic_cast<k_FileListBox*>(lk_Box_);
        if (lk_FileListBox_ && (!lk_AlreadySavedFileBoxes.contains(lk_FileListBox_)))
        {
            tk_YamlMap lk_BoxDescription;
            lk_BoxDescription["id"] = (qint64)lk_Box_;
            tk_YamlSequence lk_Coordinates;
            lk_Coordinates.push_back(boxLocation(lk_Box_).x());
            lk_Coordinates.push_back(boxLocation(lk_Box_).y());
            lk_BoxDescription["position"] = lk_Coordinates;
            tk_YamlSequence lk_Size;
            lk_Size.push_back(lk_Box_->rect().width());
            lk_Size.push_back(lk_Box_->rect().height());
            lk_BoxDescription["size"] = lk_Size;
            lk_BoxDescription["batchMode"] = lk_Box_->batchMode();
            tk_YamlSequence lk_Paths;
            foreach (QString ls_Path, lk_FileListBox_->filenames())
                lk_Paths.push_back(ls_Path);
            lk_BoxDescription["paths"] = lk_Paths;
            lk_InputFileListBoxes.push_back(lk_BoxDescription);
        }
    }
    lk_Description["inputFileListBoxes"] = lk_InputFileListBoxes;
    
    // now come the snippet boxes
    tk_YamlSequence lk_SnippetBoxes;
    foreach (IDesktopBox* lk_Box_, mk_Boxes)
    {
        // is it a snippet?
        k_SnippetBox* lk_SnippetBox_ = dynamic_cast<k_SnippetBox*>(lk_Box_);
        if (lk_SnippetBox_ && (!lk_AlreadySavedFileBoxes.contains(lk_SnippetBox_)))
        {
            tk_YamlMap lk_BoxDescription;
            lk_BoxDescription["id"] = (qint64)lk_Box_;
            tk_YamlSequence lk_Coordinates;
            lk_Coordinates.push_back(boxLocation(lk_Box_).x());
            lk_Coordinates.push_back(boxLocation(lk_Box_).y());
            lk_BoxDescription["position"] = lk_Coordinates;
            tk_YamlSequence lk_Size;
            lk_Size.push_back(lk_Box_->rect().width());
            lk_Size.push_back(lk_Box_->rect().height());
            lk_BoxDescription["size"] = lk_Size;
            lk_BoxDescription["text"] = lk_SnippetBox_->text();
            lk_BoxDescription["type"] = lk_SnippetBox_->fileType();
            lk_SnippetBoxes.push_back(lk_BoxDescription);
        }
    }
    lk_Description["snippetBoxes"] = lk_SnippetBoxes;
    
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


bool k_Desktop::applyPipelineDescription(tk_YamlMap ak_Description, QString as_DescriptionBasePath)
{
    QHash<IDesktopBox*, QPoint> lk_BoxPositions;
    QHash<IDesktopBox*, QSize> lk_BoxSizes;
    
    int li_ScriptCount = 0;
    if (ak_Description.contains("scriptBoxes"))
        li_ScriptCount = ak_Description["scriptBoxes"].toList().size();
    
    QProgressDialog lk_ProgressDialog("Loading pipeline...", "", 0, li_ScriptCount, mk_Proteomatic.messageBoxParent());
    lk_ProgressDialog.setCancelButton(0);
    lk_ProgressDialog.setWindowTitle("Proteomatic");
    lk_ProgressDialog.setWindowIcon(QIcon(":icons/proteomatic.png"));
    lk_ProgressDialog.setMinimumDuration(2000);

    clearAll();
    QHash<QString, IDesktopBox*> lk_BoxForId;
    QSet<k_FileListBox*> lk_BatchModeFileListBoxes;
    QHash<IScriptBox*, bool> lk_ShortIterationTagBoxes;
    QSet<QString> lk_Warnings;
    int li_Count = 0;
    // first check for unresolved dependencies and try to resolve them
    if (!ak_Description["scriptBoxes"].toList().empty())
    {
        QCoreApplication::processEvents();
        QString ls_ScriptBasePath;
        QSet<QString> lk_ScriptSet;
        foreach (QVariant lk_Item, ak_Description["scriptBoxes"].toList())
        {
            tk_YamlMap lk_BoxDescription = lk_Item.toMap();
            QString ls_Uri = lk_BoxDescription["uri"].toString();
            lk_ScriptSet << ls_Uri;
            if (ls_ScriptBasePath.isEmpty())
            {
                QString ls_CompleteUri = mk_Proteomatic.completePathForScript(ls_Uri);
                ls_ScriptBasePath = QFileInfo(ls_CompleteUri).absolutePath();
            }
        }
        QString ls_Response = mk_Proteomatic.syncRuby((QStringList() << 
            QFileInfo(QDir(ls_ScriptBasePath), "helper/get-unresolved-dependencies.rb").absoluteFilePath() << 
            "--extToolsPath" << mk_Proteomatic.externalToolsPath()) + lk_ScriptSet.toList());
        tk_YamlMap lk_Map = k_Yaml::parseFromString(ls_Response).toMap();
        if (!lk_Map.empty())
        {
            QStringList lk_MissingTools;
            foreach (QVariant lk_Tool, lk_Map.values())
                lk_MissingTools << lk_Tool.toString();
            qSort(lk_MissingTools);
            QString ls_MissingTools = lk_MissingTools.join(", ");
            int li_Result = mk_Proteomatic.showMessageBox("Unresolved dependencies", "This pipeline requires the following external tools that are currently not installed:\n\n"
                + ls_MissingTools + "\n\nWould you like to install them now?", ":/icons/package-x-generic.png", QMessageBox::Yes | QMessageBox::Cancel,
                QMessageBox::Yes, QMessageBox::Cancel, QString(), QString(), "Install");
            if (li_Result == QMessageBox::Yes)
            {
                bool lb_Flag = mk_Proteomatic.syncShowRuby((QStringList() << 
                    QFileInfo(QDir(ls_ScriptBasePath), "helper/resolve-dependencies.rb").absoluteFilePath() << 
                    "--extToolsPath" << mk_Proteomatic.externalToolsPath()) + lk_Map.keys(), "Installing external tools");
                if (!lb_Flag)
                {
                    clearAll();
                    setHasUnsavedChanges(false);
                    return false;
                }
            }
            else
            {
                clearAll();
                setHasUnsavedChanges(false);
                return false;
            }                
        }
    }
    
    // now load the scripts
    foreach (QVariant lk_Item, ak_Description["scriptBoxes"].toList())
    {
        QCoreApplication::processEvents();
        tk_YamlMap lk_BoxDescription = lk_Item.toMap();
        QString ls_Uri = lk_BoxDescription["uri"].toString();
        QString ls_Id = lk_BoxDescription["id"].toString();
        QString ls_CompleteUri = mk_Proteomatic.completePathForScript(ls_Uri);
        tk_YamlMap lk_OutputBoxes = lk_BoxDescription["activeOutputFiles"].toMap();
        tk_YamlMap lk_InputProxyBoxes = lk_BoxDescription["inputProxyBoxes"].toMap();
        IDesktopBox* lk_Box_ = addScriptBox(ls_CompleteUri, false, false);
        if (!lk_Box_)
        {
            // loading a script failed, now cancel this whole thing
            QString ls_Title = ls_Uri;
            mk_Proteomatic.showMessageBox("Error", 
                QString("While trying to load the pipeline, the script '%1' could not be loaded.").arg(ls_Title),
                ":icons/dialog-warning.png");
            clearAll();
            setHasUnsavedChanges(false);
            return false;
        }
        if (lk_Box_)
        {
            IScriptBox* lk_ScriptBox_ = dynamic_cast<IScriptBox*>(lk_Box_);
            
            QCoreApplication::processEvents();
            
            if (lk_ScriptBox_)
            {
                QString ls_OutputDirectory = lk_BoxDescription["outputDirectory"].toString();
                if (!ls_OutputDirectory.isEmpty())
                {
                    if (!QDir(ls_OutputDirectory).exists())
                    {
                        ls_OutputDirectory = QDir::cleanPath(as_DescriptionBasePath + "/" + ls_OutputDirectory);
                        if (!QDir(ls_OutputDirectory).exists())
                            lk_Warnings << "output-directory-gone";
                        else
                            lk_ScriptBox_->setBoxOutputDirectory(ls_OutputDirectory);
                    }
                    else
                        lk_ScriptBox_->setBoxOutputDirectory(ls_OutputDirectory);
                }
                lk_ScriptBox_->setBoxOutputPrefix(lk_BoxDescription["outputPrefix"].toString());
                lk_BoxForId[ls_Id] = lk_Box_;
                tk_YamlMap lk_Parameters = lk_BoxDescription["parameters"].toMap();
                foreach (QString ls_Key, lk_Parameters.keys())
                    lk_ScriptBox_->script()->setParameter(ls_Key, lk_Parameters[ls_Key].toString());
                if (lk_BoxDescription.contains("expanded"))
                    lk_ScriptBox_->setExpanded(lk_BoxDescription["expanded"].toBool());
                if (lk_BoxDescription["size"].toList().size() == 2)
                {
                    tk_YamlSequence lk_Size = lk_BoxDescription["size"].toList();
                    lk_BoxSizes[lk_Box_] = QSize(lk_Size[0].toInt(), lk_Size[1].toInt());
                    dynamic_cast<k_DesktopBox*>(lk_Box_)->resize(lk_Size[0].toInt(), lk_Size[1].toInt());
                }
                tk_YamlSequence lk_Position = lk_BoxDescription["position"].toList();
                lk_BoxPositions[lk_Box_] = QPoint(lk_Position[0].toInt(), lk_Position[1].toInt());
                moveBoxTo(lk_Box_, QPoint(lk_Position[0].toInt(), lk_Position[1].toInt()));
                bool lb_ShortTags = false;
                if (lk_BoxDescription["shortIterationTags"].toString() == "yes" ||
                    lk_BoxDescription["shortIterationTags"].toString() == "true")
                    lb_ShortTags = true;
                lk_ShortIterationTagBoxes[lk_ScriptBox_] = lb_ShortTags;
                // fix output boxes
                foreach (QString ls_Key, lk_ScriptBox_->script()->outputFileKeys())
                {
                    lk_ScriptBox_->setOutputFileActivated(ls_Key, lk_OutputBoxes.contains(ls_Key));
                    if (lk_OutputBoxes.contains(ls_Key))
                    {
                        IDesktopBox* lk_OutputBox_ = lk_ScriptBox_->boxForOutputFileKey(ls_Key);
                        tk_YamlMap lk_OutBoxDescription = lk_OutputBoxes[ls_Key].toMap();
                        lk_BoxForId[lk_OutBoxDescription["id"].toString()] = lk_OutputBox_;
                        if (lk_OutBoxDescription["size"].toList().size() == 2)
                        {
                            tk_YamlSequence lk_Size = lk_OutBoxDescription["size"].toList();
                            lk_BoxSizes[lk_OutputBox_] = QSize(lk_Size[0].toInt(), lk_Size[1].toInt());
                            dynamic_cast<k_DesktopBox*>(lk_OutputBox_)->resize(lk_Size[0].toInt(), lk_Size[1].toInt());
                        }
                        tk_YamlSequence lk_Position = lk_OutBoxDescription["position"].toList();
                        lk_BoxPositions[lk_OutputBox_] = QPoint(lk_Position[0].toInt(), lk_Position[1].toInt());
                        moveBoxTo(lk_OutputBox_, QPoint(lk_Position[0].toInt(), lk_Position[1].toInt()));
                        if (lk_OutBoxDescription["batchMode"].toString() == "yes" || 
                            lk_OutBoxDescription["batchMode"].toString() == "true")
                            lk_BatchModeFileListBoxes << dynamic_cast<k_FileListBox*>(lk_OutputBox_);
                    }
                }
                // fix converter out box
                if (lk_ScriptBox_->script()->type() == r_ScriptType::Converter && lk_BoxDescription.contains("converterOutputFileBoxPosition"))
                {
                    if (lk_BoxDescription["size"].toList().size() == 2)
                    {
                        tk_YamlSequence lk_Size = lk_BoxDescription["size"].toList();
                        lk_BoxSizes[lk_Box_->outgoingBoxes().toList().first()] = QSize(lk_Size[0].toInt(), lk_Size[1].toInt());
                        dynamic_cast<k_DesktopBox*>(lk_Box_->outgoingBoxes().toList().first())->resize(lk_Size[0].toInt(), lk_Size[1].toInt());
                    }
                    tk_YamlSequence lk_Position = lk_BoxDescription["converterOutputFileBoxPosition"].toList();
                    lk_BoxPositions[lk_Box_->outgoingBoxes().toList().first()] = QPoint(lk_Position[0].toInt(), lk_Position[1].toInt());
                    moveBoxTo(lk_Box_->outgoingBoxes().toList().first(), QPoint(lk_Position[0].toInt(), lk_Position[1].toInt()));
                    lk_BoxForId[lk_BoxDescription["converterOutputFileBoxId"].toString()] = lk_Box_->outgoingBoxes().toList().first();
                    if (lk_BoxDescription["converterOutputFileBoxBatchMode"].toString() == "yes" || 
                        lk_BoxDescription["converterOutputFileBoxBatchMode"].toString() == "true")
                        lk_BatchModeFileListBoxes << dynamic_cast<k_FileListBox*>(lk_Box_->outgoingBoxes().toList().first());
                }
                // fix input proxy boxes
                foreach (IDesktopBox* lk_ProxyDesktopBox_, dynamic_cast<IDesktopBox*>(lk_ScriptBox_)->incomingBoxes())
                {
                    k_InputGroupProxyBox* lk_ProxyBox_ = dynamic_cast<k_InputGroupProxyBox*>(lk_ProxyDesktopBox_);
                    if (lk_ProxyBox_)
                    {
                        QString ls_Key = lk_ProxyBox_->groupKey();
                        if (lk_InputProxyBoxes.contains(ls_Key))
                        {
                            tk_YamlMap lk_BoxDescription = lk_InputProxyBoxes[ls_Key].toMap();
                            lk_BoxForId[lk_BoxDescription["id"].toString()] = lk_ProxyBox_;
                            if (lk_BoxDescription["size"].toList().size() == 2)
                            {
                                tk_YamlSequence lk_Size = lk_BoxDescription["size"].toList();
                                lk_BoxSizes[lk_ProxyBox_] = QSize(lk_Size[0].toInt(), lk_Size[1].toInt());
                                dynamic_cast<k_DesktopBox*>(lk_ProxyBox_)->resize(lk_Size[0].toInt(), lk_Size[1].toInt());
                            }
                            tk_YamlSequence lk_Position = lk_BoxDescription["position"].toList();
                            lk_BoxPositions[lk_ProxyBox_] = QPoint(lk_Position[0].toInt(), lk_Position[1].toInt());
                            moveBoxTo(lk_ProxyBox_, QPoint(lk_Position[0].toInt(), lk_Position[1].toInt()));
                        }
                    }
                }
            }
        }
        ++li_Count;
        lk_ProgressDialog.setValue(li_Count);
    }
    foreach (QVariant lk_Item, ak_Description["inputFileListBoxes"].toList())
    {
        QCoreApplication::processEvents();
        tk_YamlMap lk_BoxDescription = lk_Item.toMap();
        QString ls_Id = lk_BoxDescription["id"].toString();
        lk_BoxForId[ls_Id] = addInputFileListBox(false);
        if (lk_BoxDescription["size"].toList().size() == 2)
        {
            tk_YamlSequence lk_Size = lk_BoxDescription["size"].toList();
            lk_BoxSizes[lk_BoxForId[ls_Id]] = QSize(lk_Size[0].toInt(), lk_Size[1].toInt());
            dynamic_cast<k_DesktopBox*>(lk_BoxForId[ls_Id])->resize(lk_Size[0].toInt(), lk_Size[1].toInt());
        }
        tk_YamlSequence lk_Position = lk_BoxDescription["position"].toList();
        lk_BoxPositions[lk_BoxForId[ls_Id]] = QPoint(lk_Position[0].toInt(), lk_Position[1].toInt());
        moveBoxTo(lk_BoxForId[ls_Id], QPoint(lk_Position[0].toInt(), lk_Position[1].toInt()));
        tk_YamlSequence lk_Paths = lk_BoxDescription["paths"].toList();
        foreach (QVariant lk_Path, lk_Paths)
        {
            QString ls_Path = lk_Path.toString();
            // if the file does not exist, try a path relative to the pipeline file
            if (!QFileInfo(ls_Path).exists())
                ls_Path = as_DescriptionBasePath + "/" + ls_Path;
            dynamic_cast<k_FileListBox*>(lk_BoxForId[ls_Id])->addPath(ls_Path);
        }
        if (lk_BoxDescription["batchMode"].toString() == "yes" || 
            lk_BoxDescription["batchMode"].toString() == "true")
            lk_BatchModeFileListBoxes << dynamic_cast<k_FileListBox*>(lk_BoxForId[ls_Id]);
    }
    foreach (QVariant lk_Item, ak_Description["snippetBoxes"].toList())
    {
        QCoreApplication::processEvents();
        tk_YamlMap lk_BoxDescription = lk_Item.toMap();
        QString ls_Id = lk_BoxDescription["id"].toString();
        lk_BoxForId[ls_Id] = addSnippetBox(false);
        if (lk_BoxDescription["size"].toList().size() == 2)
        {
            tk_YamlSequence lk_Size = lk_BoxDescription["size"].toList();
            lk_BoxSizes[lk_BoxForId[ls_Id]] = QSize(lk_Size[0].toInt(), lk_Size[1].toInt());
            dynamic_cast<k_DesktopBox*>(lk_BoxForId[ls_Id])->resize(lk_Size[0].toInt(), lk_Size[1].toInt());
        }
        tk_YamlSequence lk_Position = lk_BoxDescription["position"].toList();
        lk_BoxPositions[lk_BoxForId[ls_Id]] = QPoint(lk_Position[0].toInt(), lk_Position[1].toInt());
        moveBoxTo(lk_BoxForId[ls_Id], QPoint(lk_Position[0].toInt(), lk_Position[1].toInt()));
        
        dynamic_cast<k_SnippetBox*>(lk_BoxForId[ls_Id])->setText(lk_BoxDescription["text"].toString());
        dynamic_cast<k_SnippetBox*>(lk_BoxForId[ls_Id])->setFileType(lk_BoxDescription["type"].toString());
    }
    foreach (QVariant lk_Item, ak_Description["connections"].toList())
    {
        tk_YamlSequence lk_Pair = lk_Item.toList();
        if (lk_BoxForId.contains(lk_Pair[0].toString()) && lk_BoxForId.contains(lk_Pair[1].toString()))
            connectBoxes(lk_BoxForId[lk_Pair[0].toString()], lk_BoxForId[lk_Pair[1].toString()]);
    }
    
    clearSelection();
    
    // set batch mode for input file lists
    foreach (k_FileListBox* lk_Box_, lk_BatchModeFileListBoxes)
        lk_Box_->setBatchMode(true);
    
    // set batch mode for output file lists, 
    // this may take several rounds
    // :TODO: do not run into infinte loop if this fails
    // (it might fail, for example, if emitting signals does not
    // lead to slot being called immediately)
    while (true)
    {
        QCoreApplication::processEvents();
        bool lb_AllBatchModeOk = true;
        foreach (k_FileListBox* lk_Box_, lk_BatchModeFileListBoxes)
        {
            if (!lk_Box_)
                continue;
            lk_Box_->setBatchMode(true);
            if (!lk_Box_->batchMode())
                lb_AllBatchModeOk = false;
        }
        if (lb_AllBatchModeOk)
            break;
    }
    
    // set 'use short iteration tags' options
    foreach (IScriptBox* lk_ScriptBox_, lk_ShortIterationTagBoxes.keys())
        lk_ScriptBox_->setUseShortIterationTags(lk_ShortIterationTagBoxes[lk_ScriptBox_]);
    
    // select first script box
    foreach (IDesktopBox* lk_Box_, boxesByTopologicalOrder())
    {
        IScriptBox* lk_ScriptBox_ = dynamic_cast<IScriptBox*>(lk_Box_);
        if (lk_ScriptBox_)
        {
            setCurrentScriptBox(lk_ScriptBox_);
            break;
        }
    }
    
    resetCachedContent();
    animateAdjustView(true, QSet<IDesktopBox*>(), false);
    
    redraw();
    setHasUnsavedChanges(false);
//     emit showAllRequested();

    foreach (IDesktopBox* lk_Box_, lk_BoxSizes.keys())
        dynamic_cast<k_DesktopBox*>(lk_Box_)->resize(lk_BoxSizes[lk_Box_]);
    foreach (IDesktopBox* lk_Box_, lk_BoxPositions.keys())
        moveBoxTo(lk_Box_, lk_BoxPositions[lk_Box_]);
    
    // handle warnings
    if (!lk_Warnings.empty())
    {
        QString ls_Warning = "<b>There were problems while loading the pipeline.</b><br /><br />";
        if (lk_Warnings.contains("output-directory-gone"))
            ls_Warning += "The output directory could not be set for one or more scripts.<br/>";
        mk_Proteomatic.showMessageBox("Load pipeline", ls_Warning, ":icons/emblem-important.png");
    }
    return true;
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


void k_Desktop::bringBoxToFront(IDesktopBox* ak_Box_)
{
    // if z values get too high, reset all boxes z values
    // (but keep their order)
    if (md_BoxZ > 100000.0)
    {
        QMultiMap<double, IDesktopBox*> lk_BoxStack;
        foreach (IDesktopBox* lk_Box_, mk_Boxes)
            lk_BoxStack.insert(mk_ProxyWidgetForBox[lk_Box_]->zValue(), lk_Box_);
        md_BoxZ = 0.0;
        QMultiMap<double, IDesktopBox*>::const_iterator lk_Iter = lk_BoxStack.constBegin();
        for (; lk_Iter != lk_BoxStack.constEnd(); ++lk_Iter)
        {
            mk_ProxyWidgetForBox[lk_Iter.value()]->setZValue(md_BoxZ);
            md_BoxZ += 0.01;
        }
    }
    md_BoxZ += 0.01;
    mk_ProxyWidgetForBox[ak_Box_]->setZValue(md_BoxZ);
}


QList<IDesktopBox*> k_Desktop::boxesByTopologicalOrder()
{
    QMultiMap<int, IDesktopBox*> lk_BoxesByTopologicalIndex;
    foreach (IDesktopBox* lk_Box_, mk_Boxes)
        lk_BoxesByTopologicalIndex.insert(lk_Box_->topologicalIndex(), lk_Box_);
    return lk_BoxesByTopologicalIndex.values();
}


void k_Desktop::saveBoxPositions()
{
    mk_SavedBoxPositions.clear();
    foreach (IDesktopBox* lk_Box_, mk_Boxes)
        mk_SavedBoxPositions[lk_Box_] = boxLocation(lk_Box_);
}


void k_Desktop::restoreBoxPositions()
{
    foreach (IDesktopBox* lk_Box_, mk_SavedBoxPositions.keys())
    {
        if (mk_Boxes.contains(lk_Box_))
            moveBoxTo(lk_Box_, mk_SavedBoxPositions[lk_Box_]);
    }
    mk_SavedBoxPositions.clear();
}


void k_Desktop::bringBoxToFrontSender()
{
    QGraphicsProxyWidget* lk_ProxyWidget_ = dynamic_cast<QGraphicsProxyWidget*>(sender());
    if (lk_ProxyWidget_)
    {
        IDesktopBox* lk_Box_ = dynamic_cast<IDesktopBox*>(lk_ProxyWidget_->widget());
        if (lk_Box_)
            bringBoxToFront(lk_Box_);
    }
}


void k_Desktop::invalidate()
{
    mb_GlobalUpdateRequested = true;
    emit requestGlobalUpdate();
}


void k_Desktop::clearAll()
{
    while (!mk_Boxes.empty())
        removeBox(mk_Boxes.toList().first());
    md_Scale = 1.0;
    centerOn(QPointF(0.0, 0.0));
    QMatrix lk_Matrix = this->matrix();
    lk_Matrix.setMatrix(md_Scale, lk_Matrix.m12(), lk_Matrix.m21(), md_Scale, lk_Matrix.dx(), lk_Matrix.dy());
    this->setMatrix(lk_Matrix);
}


void k_Desktop::refresh()
{
    foreach (IDesktopBox* lk_Box_, mk_Boxes)
        mk_BoxesMarkedForUpdate.insert(lk_Box_);

    globalUpdate();

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
    refresh();
    mb_UseFileTrackerIfAvailable = ab_UseFileTrackingIfAvailable;
    // collect all script boxes
    mk_RemainingScriptBoxes.clear();
    mk_RemainingScriptBoxIterationKeys.clear();
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
    
    // check whether there are duplicate output files
    QHash<QString, IDesktopBox*> lk_OutputFilesForFileBox;
    IDesktopBox* lk_DuplicateOutputFilesBox0_ = NULL;
    IDesktopBox* lk_DuplicateOutputFilesBox1_ = NULL;
    QString ls_DuplicatePath;
    foreach (IDesktopBox* lk_Box_, mk_Boxes)
    {
        // make sure it's not an input file list box here,
        // because there duplicate files are allowed
        if (!lk_Box_->incomingBoxes().empty())
        {
            IFileBox* lk_FileBox_ = dynamic_cast<IFileBox*>(lk_Box_);
            if (lk_FileBox_)
            {
                foreach (QString ls_Path, lk_FileBox_->filenames())
                {
                    if (lk_OutputFilesForFileBox.contains(ls_Path))
                    {
                        lk_DuplicateOutputFilesBox0_ = lk_OutputFilesForFileBox[ls_Path];
                        lk_DuplicateOutputFilesBox1_ = lk_Box_;
                        ls_DuplicatePath = ls_Path;
                        break;
                    }
                    lk_OutputFilesForFileBox[ls_Path] = lk_Box_;
                }
            }
            if (lk_DuplicateOutputFilesBox0_)
                break;
        }
    }
    
    if (lk_DuplicateOutputFilesBox0_)
    {
        clearSelection();
        mk_SelectedBoxes << lk_DuplicateOutputFilesBox0_;
        mk_SelectedBoxes << lk_DuplicateOutputFilesBox1_;
        
        redraw();
        
        mk_Proteomatic.showMessageBox("Duplicate output filenames", 
                                      QString("<b>An output filename appears more than once.</b><br />The file boxes containing the offending filenames have been selected.<br />You can solve this problem by specifying a different output directory or prefix for one of the scripts involved."),
                                      ":/icons/dialog-warning.png");
        return;
    }
    
    // check whether output files are aleady there
    QList<IScriptBox*> lk_ScriptBoxesWithExistingOutputFiles;
    
    foreach (IScriptBox* lk_Box_, mk_RemainingScriptBoxes)
        if (lk_Box_->hasExistingOutputFilesForAllIterations())
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
    {
        foreach (QString ls_Tag, lk_Box_->iterationKeys())
        {
            if (lk_Box_->iterationHasNoExistingOutputFiles(ls_Tag))
                mk_RemainingScriptBoxIterationKeys[lk_Box_] << ls_Tag;
        }
    }

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
    animateAdjustView();
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
    foreach (IDesktopBox* lk_Box_, boxesByTopologicalOrder())
    {
        k_ScriptBox* lk_KScriptBox_ = dynamic_cast<k_ScriptBox*>(lk_Box_);
        if (lk_KScriptBox_ && lk_KScriptBox_->script()->type() != r_ScriptType::Converter)
        {
            lk_KScriptBox_->proposePrefixButtonClicked(false);
            globalUpdate();
        }
    }
}


void k_Desktop::updatePanMode()
{
    if (mk_PipelineMainWindow.panMode())
    {
        setDragMode(QGraphicsView::ScrollHandDrag);
        unsetCursor();
    }
    else
    {
        setDragMode(QGraphicsView::NoDrag);
        setCursor(mk_LassoCursor);
    }
}


void k_Desktop::markBoxForUpdate()
{
    IDesktopBox* lk_Box_ = dynamic_cast<IDesktopBox*>(sender());
    if (lk_Box_)
    {
//         printf("marking box for update: %p\n", lk_Box_);
        mk_BoxesMarkedForUpdate.insert(lk_Box_);
    }
}


void k_Desktop::globalUpdate()
{
    // return immediately if nothing to update
    if (mk_BoxesMarkedForUpdate.empty() && (!mb_GlobalUpdateRequested))
        return;
    
    mk_BoxesMarkedForUpdate &= mk_Boxes;
    
    QMultiMap<int, IDesktopBox*> lk_BoxesByTopologicalIndex;
    foreach (IDesktopBox* lk_Box_, mk_BoxesMarkedForUpdate)
        lk_BoxesByTopologicalIndex.insert(lk_Box_->topologicalIndex(), lk_Box_);
    
    QMultiMap<int, IDesktopBox*>::const_iterator lk_Iter = lk_BoxesByTopologicalIndex.constBegin();
    for (; lk_Iter != lk_BoxesByTopologicalIndex.constEnd(); ++lk_Iter)
    {
        IDesktopBox* lk_Box_ = lk_Iter.value();
//         printf("[%d] updating %p\n", lk_Iter.key(), lk_Iter.value());
        lk_Box_->update();
    }
    
    // always update the desktop
    //if (mb_GlobalUpdateRequested)
    {
        mk_BatchBoxes.clear();
        foreach (IDesktopBox* lk_Box_, mk_Boxes)
        {
            if (lk_Box_->batchMode())
                mk_BatchBoxes.insert(lk_Box_);
            else
                mk_BatchBoxes.remove(lk_Box_);
        }
        redrawBatchFrame();
        
        // update all arrows... :TODO: only update necessary arrows
        foreach (IDesktopBox* lk_Box_, mk_Boxes)
        {
            foreach (QGraphicsPathItem* lk_Arrow_, mk_ArrowsForBox[lk_Box_])
                updateArrow(lk_Arrow_);
        }
    }
    
    mk_BoxesMarkedForUpdate.clear();
    mb_GlobalUpdateRequested = false;

    mb_HasUnsavedChanges = true;
    mk_PipelineMainWindow.toggleUi();
}


void k_Desktop::boxMovedOrResized(QPoint /*ak_Delta*/)
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
    
    if (!lk_Box_)
        return;
    
    bringBoxToFront(lk_Box_);
    
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
    updateUserArrow(mapToScene(mapFromGlobal(QCursor::pos())));
}


void k_Desktop::arrowReleased()
{
    if (mk_ArrowStartBox_ && mk_ArrowEndBox_)
        connectBoxes(mk_ArrowStartBox_, mk_ArrowEndBox_);
    
    if (mk_ArrowStartBox_ && !mk_ArrowEndBox_)
    {
        // show scripts menu and insert a script box here
        if ((!boxAt(mk_ArrowEndPoint)) && (!mk_Proteomatic.availableScripts().empty()))
        {
            mk_ArrowStartBoxAutoConnect_ = mk_ArrowStartBox_;
            QStringList lk_InputPaths;
            IFileBox* lk_FileBox_ = dynamic_cast<IFileBox*>(mk_ArrowStartBox_);
            if (lk_FileBox_)
            {
                foreach (QString ls_Path, lk_FileBox_->filenames())
                    lk_InputPaths << ls_Path;
                mk_Proteomatic.highlightScriptsMenu(lk_InputPaths);
                k_SearchMenu* lk_SearchMenu_ = dynamic_cast<k_SearchMenu*>(mk_Proteomatic.proteomaticScriptsMenu());
                if (lk_SearchMenu_)
                    lk_SearchMenu_->setInputFilenames(lk_InputPaths);
            }
            QAction* lk_Action_ = mk_Proteomatic.proteomaticScriptsMenu()->exec(mapFromScene(mk_ArrowEndPoint) + mapToGlobal(pos()) + QPoint(4, 4));
            if (lk_FileBox_)
            {
                mk_Proteomatic.highlightScriptsMenu();
                k_SearchMenu* lk_SearchMenu_ = dynamic_cast<k_SearchMenu*>(mk_Proteomatic.proteomaticScriptsMenu());
                if (lk_SearchMenu_)
                    lk_SearchMenu_->setInputFilenames(QStringList());
            }
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
    if (((dynamic_cast<IFileBox*>(lk_ArrowStartBox_)) && lk_ArrowStartBox_->batchMode()) ||
        ((dynamic_cast<k_InputGroupProxyBox*>(lk_ArrowStartBox_)) && lk_ArrowStartBox_->batchMode()))
    {
        if (!(dynamic_cast<IScriptBox*>(lk_ArrowEndBox_) && (dynamic_cast<IScriptBox*>(lk_ArrowEndBox_)->script()->type() == r_ScriptType::Converter)))
        {
            lk_Pen = QPen(TANGO_BUTTER_2);
            lk_Brush = QBrush(TANGO_BUTTER_2);
        }
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
    {
        if (dynamic_cast<IDesktopBox*>(mk_Arrows[lk_Arrow_].first) && mk_Arrows[lk_Arrow_].first->batchMode() &&
            dynamic_cast<IDesktopBox*>(mk_Arrows[lk_Arrow_].second) && mk_Arrows[lk_Arrow_].second->batchMode())
            lk_Path = lk_Path.united(grownPathForBatchConnector(mk_Arrows[lk_Arrow_].first, mk_Arrows[lk_Arrow_].second, 10));
        else
            lk_Path = lk_Path.united(grownPathForArrow(lk_Arrow_, 3));
    }
    
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
        {
            mk_SelectedArrows.remove(lk_Arrow_);
            disconnectBoxes(mk_Arrows[lk_Arrow_].first, mk_Arrows[lk_Arrow_].second);
        }
    }
    
    while (!mk_SelectedBoxes.empty())
    {
        IDesktopBox* lk_First_ = mk_SelectedBoxes.toList().first();
        // skip this box if it's protected from deletion
        if (lk_First_->protectedFromUserDeletion())
            mk_SelectedBoxes.remove(lk_First_);
        else
        {
            mk_SelectedBoxes.remove(lk_First_);
            removeBox(lk_First_);
        }
    }
    
    clearSelection();
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
        bool lb_FirstIsFileBox = dynamic_cast<IFileBox*>(lk_Box_);
        bool lb_FirstIsProxyBox = dynamic_cast<k_InputGroupProxyBox*>(lk_Box_);
        if (lb_FirstIsFileBox || lb_FirstIsProxyBox)
        {
            foreach (IDesktopBox* lk_PeerBox_, lk_Box_->outgoingBoxes())
            {
                bool lb_SecondIsScriptBox = dynamic_cast<IScriptBox*>(lk_PeerBox_);
                bool lb_SecondIsProxyBox = dynamic_cast<k_InputGroupProxyBox*>(lk_PeerBox_);
                if (!(lb_SecondIsScriptBox || lb_SecondIsProxyBox))
                    continue;
                if (lb_SecondIsScriptBox)
                {
                    if ((dynamic_cast<IScriptBox*>(lk_PeerBox_))->script()->type() == r_ScriptType::Converter)
                        continue;
                }
                QPainterPath lk_SubPath;
                lk_SubPath = grownPathForBatchConnector(lk_Box_, lk_PeerBox_);
                lk_Path = lk_Path.united(lk_SubPath);
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
    mk_Proteomatic.touchScriptsLockFile();
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
    if (lk_ScriptBox_ == NULL)
        return;
    if (ai_ExitCode == 0)
    {
        lk_ScriptBox_->addOutput("\n-----------------------------------\n");
        lk_ScriptBox_->addOutput(QString("Script finished successfully.\n"));
        // do next iteration if there are still some left from sender
        lk_ScriptBox_->showOutputFileBox(true);

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
        lk_ScriptBox_->addOutput("\n-----------------------------------\n");
        lk_ScriptBox_->addOutput(QString("Script failed with exit code %1\n").arg(ai_ExitCode));
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


void k_Desktop::animationTimeout()
{
    if (!mb_Animating)
        return;
    
    double t = mk_StopWatch.get_Time() / 0.5;
    if (!mk_Proteomatic.stringToBool(mk_Proteomatic.getConfiguration(CONFIG_ANIMATION).toString()))
        t = 1.1;
    
    if (t > 1.0)
    {
        t = 1.0;
        mk_AnimationTimer.stop();
        mb_Animating = false;
    }
    t = pow(t, 0.3);
    md_Scale = (md_AnimationEndScale - md_AnimationStartScale) * t + md_AnimationStartScale;
    QPointF lk_Center = (mk_AnimationEndCenter - mk_AnimationStartCenter) * t + mk_AnimationStartCenter;
    centerOn(lk_Center);
    QMatrix lk_Matrix = this->matrix();
    lk_Matrix.setMatrix(md_Scale, lk_Matrix.m12(), lk_Matrix.m21(), md_Scale, lk_Matrix.dx(), lk_Matrix.dy());
    this->setMatrix(lk_Matrix);
}


void k_Desktop::animateAdjustView(bool ab_ZoomIn, QSet<IDesktopBox*> ak_FocusOn, bool ab_Animate)
{
    if (mk_Boxes.empty())
        return;

    double lf_SceneMargin = mapToScene(10, 0).x() - mapToScene(0, 0).x();
    // determine bounding box
    QRectF lk_Rect;
    if (ak_FocusOn.empty())
        ak_FocusOn = mk_Boxes;
    foreach (IDesktopBox* lk_Box_, ak_FocusOn)
        lk_Rect = lk_Rect.united(lk_Box_->rect());
    lk_Rect.adjust(-lf_SceneMargin, -lf_SceneMargin, lf_SceneMargin, lf_SceneMargin);
    
    // if everything's visible: return
    QRectF lk_ViewRect(mapToScene(QPoint(0.0, 0.0)), mapToScene(QPoint(frameRect().width() - 2, frameRect().height() - 2)));
    if ((!ab_ZoomIn) && lk_ViewRect.contains(lk_Rect))
        return;

    md_AnimationStartScale = md_Scale;
    md_AnimationEndScale = md_AnimationStartScale;
    mk_AnimationStartCenter = lk_ViewRect.center();
    
    // reset scaling to 1.0
    if (ab_ZoomIn)
        md_Scale = 1.0;
    
    // center
    //centerOn(lk_Rect.center());
    if (ab_ZoomIn)
    {
        mk_AnimationEndCenter = lk_Rect.center();
    }
    else
    {
        double x = 0.0;
        double y = 0.0;
        if (lk_Rect.right() > lk_ViewRect.right())
            x -= lk_Rect.right() - lk_ViewRect.right();
        if (lk_Rect.left() < lk_ViewRect.left())
            x -= lk_Rect.left() - lk_ViewRect.left();
        if (lk_Rect.bottom() > lk_ViewRect.bottom())
            y -= lk_Rect.bottom() - lk_ViewRect.bottom();
        if (lk_Rect.top() < lk_ViewRect.top())
            y -= lk_Rect.top() - lk_ViewRect.top();
        mk_AnimationEndCenter = mk_AnimationStartCenter - QPointF(x, y);
    }
    
    // adjust scaling
    QPointF lk_EndHalfSize = QPointF((double)frameRect().width() / md_Scale, 
                                     (double)frameRect().height() / md_Scale) / 2.0;
    QPointF lk_A = mk_AnimationEndCenter - lk_EndHalfSize;
    QPointF lk_B = mk_AnimationEndCenter + lk_EndHalfSize;
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
        //this->setMatrix(lk_Matrix);
    }
    md_AnimationEndScale = md_Scale;
    md_Scale = md_AnimationStartScale;
    if (ab_Animate)
    {
        mk_AnimationTimer.setSingleShot(false);
        mk_StopWatch.reset();
        mb_Animating = true;
        mk_AnimationTimer.start(20);
    }
    else
    {
        md_Scale = md_AnimationEndScale;
        QPointF lk_Center = mk_AnimationEndCenter;
        centerOn(lk_Center);
        QMatrix lk_Matrix = this->matrix();
        lk_Matrix.setMatrix(md_Scale, lk_Matrix.m12(), lk_Matrix.m21(), md_Scale, lk_Matrix.dx(), lk_Matrix.dy());
        this->setMatrix(lk_Matrix);
    }
}


void k_Desktop::updateWatchedDirectories()
{
    QSet<QString> lk_Paths;
    // collect paths from input file list boxes
    foreach (IDesktopBox* lk_Box_, mk_Boxes)
    {
        k_FileListBox* lk_FileListBox_ = dynamic_cast<k_FileListBox*>(lk_Box_);
        if (lk_FileListBox_)
        {
            foreach (QString ls_Path, lk_FileListBox_->filenames())
                lk_Paths << QFileInfo(ls_Path).absoluteDir().absolutePath();
        }
        IScriptBox* lk_ScriptBox_ = dynamic_cast<IScriptBox*>(lk_Box_);
        if (lk_ScriptBox_)
            lk_Paths << lk_ScriptBox_->scriptOutputDirectory();
    }
    
    // remove old paths
    foreach (QString ls_Path, mk_FileSystemWatcher.directories())
        if (!lk_Paths.contains(ls_Path))
            mk_FileSystemWatcher.removePath(ls_Path);
        
    // add new paths
    foreach (QString ls_Path, lk_Paths)
        if (!mk_FileSystemWatcher.directories().contains(ls_Path))
            mk_FileSystemWatcher.addPath(ls_Path);
}


void k_Desktop::keyPressEvent(QKeyEvent* event)
{
    QGraphicsView::keyPressEvent(event);
    if (!event->isAccepted())
    {
        if ((event->matches(QKeySequence::Delete) || (event->key() == Qt::Key_Backspace)) && !mb_Running)
            deleteSelected();
        if (event->matches(QKeySequence::Paste) && !mb_Running)
        {
            QClipboard* lk_Clipboard_ = QApplication::clipboard();
            if (lk_Clipboard_->mimeData()->hasText())
            {
                QString ls_Text = lk_Clipboard_->text();
                if (!ls_Text.isEmpty())
                {
                    k_SnippetBox* lk_Box_ = dynamic_cast<k_SnippetBox*>(this->addSnippetBox(true));
                    lk_Box_->setText(ls_Text);
                }
            }
        }
#ifdef DEBUG
        
        if (event->text().toLower() == "d")
        {
            printf("\n");
            printf("DEBUG OUTPUT\n");
            printf("Got %d boxes.\n", mk_Boxes.size());
            foreach (IDesktopBox* lk_Box_, boxesByTopologicalOrder())
            {
                printf("%s\n", lk_Box_->description().toStdString().c_str());
                IScriptBox* lk_ScriptBox_ = dynamic_cast<IScriptBox*>(lk_Box_);
                if (lk_ScriptBox_)
                {
                    foreach (QString ls_Key, lk_ScriptBox_->iterationKeys())
                    {
                        printf("    [%s] (files: %s)\n", ls_Key.toStdString().c_str(), lk_ScriptBox_->iterationHasNoExistingOutputFiles(ls_Key) ? "no" : "yes");
                    }
                }
            }
            printf("\n");
            printf("\n");
        }
#endif
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
/*          mk_CurrentScriptBox_ = NULL;
            mk_PipelineMainWindow.setPaneLayoutWidget(NULL);*/
            redrawSelection();
            if (!mk_PipelineMainWindow.panMode())
            {
                mb_LassoSelecting = true;
                mk_LassoPath = QPainterPath();
                mk_LastLassoDevicePoint = event->pos();
                mk_LassoPath.moveTo(mapToScene(mk_LastLassoDevicePoint));
                mk_LassoGraphicsPathItem_->setPath(mk_LassoPath);
            }
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


void k_Desktop::mouseDoubleClickEvent(QMouseEvent* event)
{
    QPointF lk_Position = this->mapToScene(event->pos());
    if (mk_GraphicsScene.items(lk_Position).empty())
    {
        if ((event->modifiers() & Qt::ControlModifier) == 0)
        {
            clearSelection();
/*          mk_CurrentScriptBox_ = NULL;
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
            clearSelection();
            redrawSelection();
            if (lk_ProxyLine_)
            {
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
    QGraphicsView::mouseDoubleClickEvent(event);
}


void k_Desktop::mouseReleaseEvent(QMouseEvent* event)
{
    mb_Moving = false;
    if (mb_LassoSelecting)
    {
        clearSelection();
        foreach (IDesktopBox* lk_Box_, mk_Boxes)
        {
            if (mk_LassoPath.intersects(dynamic_cast<k_DesktopBox*>(lk_Box_)->frameGeometry()))
                mk_SelectedBoxes << lk_Box_;
        }
        redrawSelection();
        mk_LassoPath = QPainterPath();
        mk_LassoGraphicsPathItem_->setPath(mk_LassoPath);
        mb_LassoSelecting = false;
    }
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
    else if (mb_LassoSelecting)
    {
        QPoint lk_NewPoint = event->pos();
        if ((lk_NewPoint - mk_LastLassoDevicePoint).manhattanLength() > 10)
        {
            mk_LassoPath.lineTo(mapToScene(lk_NewPoint));
            mk_LastLassoDevicePoint = lk_NewPoint;
            mk_LassoGraphicsPathItem_->setPath(mk_LassoPath);
        }
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
    else if (event->modifiers() == 0)
        QGraphicsView::wheelEvent(event);
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
    
    IScriptBox* lk_ScriptBox_ = dynamic_cast<IScriptBox*>(ak_EndBox_);
    if (lk_ScriptBox_)
    {
        // make sure the arrow end box is not a script box which has ambiguous input groups
        // in that case, the files should go to one of the proxy boxes
        if (!lk_ScriptBox_->script()->ambiguousInputGroups().empty())
            return NULL;
        // don't connect if the end box is a script box that has no input files
        if (lk_ScriptBox_->script()->inputGroupKeys().empty()) 
            return NULL;
    }
    
    if (ak_StartBox_->incomingBoxes().contains(ak_EndBox_))
        return NULL;
    if (ak_StartBox_->outgoingBoxes().contains(ak_EndBox_))
        return NULL;
    
    // make sure the arrow end box is not a predecessor of the arrow start box
    if (ak_StartBox_->hasAnchestor(ak_EndBox_))
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


QPainterPath k_Desktop::grownPathForBatchConnector(IDesktopBox* ak_Box_, IDesktopBox* ak_PeerBox_, int ai_Grow)
{
    QPainterPath lk_Result;
    QPointF p0 = boxLocation(ak_Box_);
    QPointF p1 = boxLocation(ak_PeerBox_);
    QPointF p0c = p0;
    QPointF p1c = p1;
    intersectLineWithBox(p0c, p1c, ak_Box_);
    intersectLineWithBox(p0c, p1c, ak_PeerBox_);
    QPointF m0 = p0c + (p1c - p0c) * 0.1;
    QPointF m1 = p0c + (p1c - p0c) * 0.9;
    
    QSize s0 = dynamic_cast<k_DesktopBox*>(ak_Box_)->size();
    QSize s1 = dynamic_cast<k_DesktopBox*>(ak_PeerBox_)->size();
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
        lk_SubPath.moveTo(p0 + lk_Normal * (r0 + ai_Grow));
        lk_SubPath.cubicTo(m0, m1, p1 + lk_Normal * (r1 + ai_Grow));
        lk_SubPath.lineTo(p1 - lk_Normal * (r1 + ai_Grow));
        lk_SubPath.cubicTo(m1, m0, p0 - lk_Normal * (r0 + ai_Grow));
        lk_SubPath.lineTo(p0 + lk_Normal * (r0 + ai_Grow));
        lk_Result = lk_SubPath;
    }
    return lk_Result;
}


QPointF k_Desktop::findFreeSpace(QRectF ak_BoundRect, int ai_BoxCount, QRectF ak_BoxRect)
{
    QPointF lk_HalfSize = QPointF(ak_BoxRect.width() * 0.5, ak_BoxRect.height() * 0.5);
                 
    if (ai_BoxCount == 0)
        return QPointF(0.0, 0.0) - lk_HalfSize;

    QRectF lk_Rect = ak_BoundRect.adjusted(-8.0, -8.0, 8.0, 8.0);
    double p[4] = {lk_Rect.top(), lk_Rect.right(), lk_Rect.bottom(), lk_Rect.left()};
    int best = 0;
    double bestd = fabs(p[0]);
    for (int i = 1; i < 4; ++i)
    {
        double d = fabs(p[i]);
        if (d < bestd)
        {
            bestd = d;
            best = i;
        }
    }
    if (best == 0)
        return QPointF(0.0, lk_Rect.top() - lk_HalfSize.y()) - lk_HalfSize;
    if (best == 1)
        return QPointF(lk_Rect.right() + lk_HalfSize.x(), 0.0) - lk_HalfSize;
    if (best == 2)
        return QPointF(0.0, lk_Rect.bottom() + lk_HalfSize.y()) - lk_HalfSize;
    if (best == 3)
        return QPointF(lk_Rect.left() - lk_HalfSize.x(), 0.0) - lk_HalfSize;
    return QPointF() - lk_HalfSize;
}


IScriptBox* k_Desktop::pickNextScriptBox()
{
    foreach (IDesktopBox* lk_Box_, boxesByTopologicalOrder())
    {
        IScriptBox* lk_ScriptBox_ = dynamic_cast<IScriptBox*>(lk_Box_);
        if (lk_ScriptBox_ && mk_RemainingScriptBoxes.contains(lk_ScriptBox_))
            if (lk_ScriptBox_->checkReadyToGo())
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
    if (running())
        event->ignore();
    else
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
    if (running())
    {
        event->ignore();
        return;
    }
    
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
        if ((lk_Files.size() == 1) && (lk_Files.first().endsWith(QString(FILE_EXTENSION_PIPELINE))))
            // IT'S A PIPELINE!!
            mk_PipelineMainWindow.loadPipeline(lk_Files.first());
        else
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
}


Qt::DropActions k_Desktop::supportedDropActions() const
{
    return Qt::ActionMask;
}
