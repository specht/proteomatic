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
#include "IDesktopBox.h"
#include "IScriptBox.h"
#include "StopWatch.h"
#include "Yaml.h"


class k_PipelineMainWindow;
class k_Proteomatic;


typedef QPair<IDesktopBox*, IDesktopBox*> tk_BoxPair;


class k_Desktop: public QGraphicsView
{
    Q_OBJECT
public:
    k_Desktop(QWidget* ak_Parent_, k_Proteomatic& ak_Proteomatic, k_PipelineMainWindow& ak_PipelineMainWindow);
    virtual ~k_Desktop();

    virtual k_PipelineMainWindow& pipelineMainWindow() const;
    virtual QGraphicsScene& graphicsScene();
    virtual IDesktopBox* addInputFileListBox(bool ab_AutoAdjust = true);
    virtual IDesktopBox* addSnippetBox(bool ab_AutoAdjust = true);
    virtual IDesktopBox* addScriptBox(const QString& as_ScriptUri, bool ab_AutoAdjust = true, bool ab_AutoAddFileBoxIfEmpty = true);
    virtual void addBox(IDesktopBox* ak_Box_, bool ab_PlaceBox = true, bool ab_AutoAdjust = true, int ai_Width = 1, int ai_Height = 1);
    virtual void removeBox(IDesktopBox* ak_Box_);
    virtual void connectBoxes(IDesktopBox* ak_Source_, IDesktopBox* ak_Destination_);
    virtual void disconnectBoxes(IDesktopBox* ak_Source_, IDesktopBox* ak_Destination_);
    virtual void moveSelectedBoxesStart(IDesktopBox* ak_IncludeThis_);
    virtual void moveSelectedBoxes(QPointF ak_Delta);
    
    virtual void createFilenameTags(QStringList ak_Filenames, QHash<QString, QString>& ak_TagForFilename, QString& as_PrefixWithoutTags);
    virtual bool running() const;
    
    virtual bool hasBoxes();
    virtual bool hasScriptBoxes();
    virtual tk_YamlMap pipelineDescription();
    virtual bool applyPipelineDescription(tk_YamlMap ak_Description, QString as_DescriptionBasePath);
    
    virtual void setHasUnsavedChanges(bool ab_Flag);
    virtual bool hasUnsavedChanges() const;
    virtual QSet<IDesktopBox*> selectedBoxes() const;
    
    virtual bool useFileTrackerIfAvailable() const;
    virtual void bringBoxToFront(IDesktopBox* ak_Box_);
    
    virtual QList<IDesktopBox*> boxesByTopologicalOrder();
    
    virtual void saveBoxPositions();
    virtual void restoreBoxPositions();
    
public slots:
    virtual void clearAll();
    virtual void refresh();
    virtual void redraw();
    virtual void start(bool ab_UseFileTrackingIfAvailable = true);
    virtual void abort();
    virtual void showAll();
    virtual void clearPrefixForAllScripts();
    virtual void proposePrefixForAllScripts();
    virtual void updatePanMode();
    virtual void invalidate();
    
signals:
    void pipelineIdle(bool);
    void showAllRequested();
    void selectionChanged();
    void requestGlobalUpdate();
    
protected slots:
    virtual void markBoxForUpdate();
    virtual void globalUpdate();
    virtual void boxMovedOrResized(QPoint ak_Delta = QPoint());
    virtual void boxClicked(QMouseEvent* event);
    virtual void arrowPressed();
    virtual void arrowReleased();
    virtual void updateArrow(QGraphicsPathItem* ak_Arrow_);
    virtual void redrawSelection(bool ab_DontCallOthers = false);
    virtual void deleteSelected();
    virtual void redrawBatchFrame(bool ab_DontCallOthers = false);
    virtual void clearSelection();
    virtual void scriptStarted();
    virtual void scriptFinished(int ai_ExitCode);
    virtual void setCurrentScriptBox(IScriptBox* ak_ScriptBox_);
    virtual void setCurrentScriptBoxForce(IScriptBox* ak_ScriptBox_);
    virtual void animationTimeout();
    virtual void animateAdjustView(bool ab_ZoomIn = true, QSet<IDesktopBox*> ak_FocusOn = QSet<IDesktopBox*>(), bool ab_Animate = true);
    virtual void updateWatchedDirectories();
    virtual void bringBoxToFrontSender();
    
protected:
    virtual void keyPressEvent(QKeyEvent* event);
    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseDoubleClickEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);
    virtual void mouseMoveEvent(QMouseEvent* event);
    virtual void wheelEvent(QWheelEvent* event);
    virtual void updateUserArrow(QPointF ak_MousePosition);
    virtual IDesktopBox* connectionAllowed(IDesktopBox* ak_StartBox_, IDesktopBox* ak_EndBox_);
    virtual QPoint boxLocation(IDesktopBox* ak_Box_) const;
    virtual void moveBoxTo(IDesktopBox* ak_Box_, QPoint ak_Position);
    virtual IDesktopBox* boxAt(QPointF ak_Point) const;
    virtual double intersect(QPointF p0, QPointF d0, QPointF p1, QPointF d1);
    virtual void boxConnector(IDesktopBox* ak_Box0_, IDesktopBox* ak_Box1_, QPointF& ak_Point0, QPointF& ak_Point1);
    virtual QPointF intersectLineWithBox(const QPointF& ak_Point0, const QPointF& ak_Point1, IDesktopBox* ak_Box_);
    virtual void updateArrowInternal(QGraphicsPathItem* ak_Arrow_, QPointF ak_Start, QPointF ak_End);
    virtual QPainterPath grownPathForBox(IDesktopBox* ak_Box_, int ai_Grow = 0);
    virtual QPainterPath grownPathForArrow(QGraphicsPathItem* ak_Arrow_, int ai_Grow = 0);
    virtual QPainterPath grownPathForBatchConnector(IDesktopBox* ak_Box_, IDesktopBox* ak_PeerBox_, int ai_Grow = 0);
    virtual QPointF findFreeSpace(QRectF ak_BoundRect, int ai_BoxCount, QRectF ak_BoxRect);
    virtual IScriptBox* pickNextScriptBox();
    virtual QSet<IScriptBox*> incomingScriptBoxes(IDesktopBox* ak_Box_) const;
    virtual void dragEnterEvent(QDragEnterEvent* event);
    virtual void dragMoveEvent(QDragMoveEvent* event);
    virtual void dropEvent(QDropEvent* event);
    virtual Qt::DropActions supportedDropActions() const;
    
    k_Proteomatic& mk_Proteomatic;
    k_PipelineMainWindow& mk_PipelineMainWindow;
    QGraphicsScene mk_GraphicsScene;
    
    double md_Scale;
    
    QSet<IDesktopBox*> mk_Boxes;
    
    QHash<IScript*, IScriptBox*> mk_BoxForScript;
    QHash<IDesktopBox*, QGraphicsProxyWidget*> mk_ProxyWidgetForBox;

    // an arrow in the making!
    IDesktopBox* mk_ArrowStartBox_;
    IDesktopBox* mk_ArrowEndBox_;
    QGraphicsPathItem* mk_UserArrowPathItem_;
    QPointF mk_ArrowEndPoint;
    QPointF mk_ArrowDirection;
    IDesktopBox* mk_ArrowStartBoxAutoConnect_;
    
    // all arrows are kept in this hash
    QHash<QGraphicsPathItem*, tk_BoxPair> mk_Arrows;
    // secondary hash, reverse
    QHash<tk_BoxPair, QGraphicsPathItem*> mk_ArrowForBoxPair;
    // secondary hash, all arrows for each box
    QHash<IDesktopBox*, QSet<QGraphicsPathItem* > > mk_ArrowsForBox;
    // and a thick invisible line under each arrow for picking!
    QHash<QGraphicsPathItem*, QGraphicsLineItem*> mk_ArrowProxy;
    QHash<QGraphicsLineItem*, QGraphicsPathItem*> mk_ArrowForProxy;
    
    QSet<IDesktopBox*> mk_SelectedBoxes;
    QSet<QGraphicsPathItem*> mk_SelectedArrows;
    QGraphicsPathItem* mk_SelectionGraphicsPathItem_;
    
    IDesktopBox* mk_CurrentScriptBox_;
    QGraphicsPathItem* mk_CurrentScriptBoxGraphicsPathItem_;

    QSet<IDesktopBox*> mk_BatchBoxes;
    QGraphicsPathItem* mk_BatchGraphicsPathItem_;
    
    bool mb_Running;
    bool mb_Error;
    QSet<IScriptBox*> mk_RemainingScriptBoxes;
    QHash<IScriptBox*, QStringList> mk_RemainingScriptBoxIterationKeys;
    
    // mk_DeleteBoxStackSet is there to prevent recursive
    // deletion of boxes which might happen when boxes
    // depend on each other
    QSet<IDesktopBox*> mk_DeleteBoxStackSet;
    
    double md_BoxZ;
    bool mb_HasUnsavedChanges;
    QHash<IDesktopBox*, QPoint> mk_MoveSelectionStartPositions;
    QPointF mk_MoveStartPoint;
    bool mb_Moving;
    
    bool mb_UseFileTrackerIfAvailable;
    k_StopWatch mk_StopWatch;
    QTimer mk_AnimationTimer;
    QPointF mk_AnimationStartCenter;
    QPointF mk_AnimationEndCenter;
    double md_AnimationStartScale;
    double md_AnimationEndScale;
    bool mb_Animating;
    
    bool mb_LassoSelecting;
    QPainterPath mk_LassoPath;
    QPoint mk_LastLassoDevicePoint;
    QGraphicsPathItem* mk_LassoGraphicsPathItem_;
    QCursor mk_LassoCursor;
    QFileSystemWatcher mk_FileSystemWatcher;
    QSet<IDesktopBox*> mk_BoxesMarkedForUpdate;
    
    bool mb_GlobalUpdateRequested;
    QHash<IDesktopBox*, QPoint> mk_SavedBoxPositions;
};
