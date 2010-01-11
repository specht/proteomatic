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

#include "InputGroupProxyBox.h"
#include "UnclickableLabel.h"


k_InputGroupProxyBox::k_InputGroupProxyBox(k_Desktop* ak_Parent_, k_Proteomatic& ak_Proteomatic,
                                            QString as_Label, QString as_GroupKey)
    : k_DesktopBox(ak_Parent_, ak_Proteomatic, false, false)
    , ms_Label(as_Label)
    , ms_GroupKey(as_GroupKey)
{
    connect(this, SIGNAL(boxConnected(IDesktopBox*, bool)), this, SLOT(boxConnectedSlot(IDesktopBox*, bool)));
    connect(this, SIGNAL(boxDisconnected(IDesktopBox*, bool)), this, SLOT(boxDisconnectedSlot(IDesktopBox*, bool)));
    setProtectedFromUserDeletion(true);
    setupLayout();
}


k_InputGroupProxyBox::~k_InputGroupProxyBox()
{
}


const QString& k_InputGroupProxyBox::groupKey() const
{
    return ms_GroupKey;
}


QList<IFileBox*> k_InputGroupProxyBox::fileBoxes() const
{
    return mk_FileBoxes;
}


void k_InputGroupProxyBox::boxConnectedSlot(IDesktopBox* ak_Other_, bool ab_Incoming)
{
    if (ab_Incoming)
    {
        IFileBox* lk_FileBox_ = dynamic_cast<IFileBox*>(ak_Other_);
        if (lk_FileBox_)
            mk_FileBoxes.append(lk_FileBox_);
    }
}


void k_InputGroupProxyBox::boxDisconnectedSlot(IDesktopBox* ak_Other_, bool ab_Incoming)
{
    if (ab_Incoming)
    {
        IFileBox* lk_FileBox_ = dynamic_cast<IFileBox*>(ak_Other_);
        if (lk_FileBox_)
            mk_FileBoxes.removeOne(lk_FileBox_);
    }
}


void k_InputGroupProxyBox::setupLayout()
{
    QBoxLayout* lk_Layout_ = new QVBoxLayout(this);
    lk_Layout_->setContentsMargins(5, 5, 5, 5);

    QLabel* lk_Label_ = new k_UnclickableLabel(ms_Label, this);
    lk_Layout_->addWidget(lk_Label_);
}


void k_InputGroupProxyBox::update()
{
    // ---------------------------------------
    // UPDATE BATCH MODE
    // ---------------------------------------
    
    // batch mode this box if at least one incoming box is in batch mode
    bool lb_BatchMode = false;
    foreach (IDesktopBox* lk_Box_, mk_ConnectedIncomingBoxes)
    {
        if (lk_Box_->batchMode())
        {
            lb_BatchMode = true;
            break;
        }
    }
    setBatchMode(lb_BatchMode);
    
    emit changed();
}
