/*
Copyright (c) 2010 Michael Specht

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

#include "PipelineTabWidget.h"
#include "ClickableLabel.h"


k_PipelineTabWidget::k_PipelineTabWidget(QWidget* parent)
    : QTabWidget(parent)
{
    this->toggleUi();
}


k_PipelineTabWidget::~k_PipelineTabWidget()
{
}


void k_PipelineTabWidget::tabInserted(int ai_Index)
{
    QTabWidget::tabInserted(ai_Index);
    if (ai_Index > 0)
    {
        k_ClickableLabel* lk_CloseButton_ = new k_ClickableLabel(NULL);
        lk_CloseButton_->setPixmap(QPixmap(":icons/dialog-cancel.png").scaledToHeight(12, Qt::SmoothTransformation));
//         lk_CloseButton_->setFlat(true);
        lk_CloseButton_->setProperty("index", QVariant(ai_Index));
        connect(lk_CloseButton_, SIGNAL(clicked()), this, SLOT(closeButtonClicked()));
        tabBar()->setTabButton(ai_Index, QTabBar::LeftSide, lk_CloseButton_);
    }
    this->toggleUi();
}


void k_PipelineTabWidget::tabRemoved(int ai_Index)
{
    QTabWidget::tabRemoved(ai_Index);
    this->toggleUi();
}


void k_PipelineTabWidget::toggleUi()
{
    tabBar()->setVisible(count() > 1);
}


void k_PipelineTabWidget::closeButtonClicked()
{
    emit tabCloseRequested(sender()->property("index").toInt());
}
