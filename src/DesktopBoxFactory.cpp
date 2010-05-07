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

#include "DesktopBoxFactory.h"
#include "InputGroupProxyBox.h"
#include "ScriptFactory.h"
#include "ScriptBox.h"
#include "FileListBox.h"
#include "Script.h"


IDesktopBox*
k_DesktopBoxFactory::makeScriptBox(QString as_ScriptUri, k_Desktop* ak_Parent_, 
                                    k_Proteomatic& ak_Proteomatic)
{
    QSharedPointer<IScript> lk_pScript = k_ScriptFactory::makeScript(as_ScriptUri, ak_Proteomatic, false, false);
    k_Script* lk_Script_ = dynamic_cast<k_Script*>(lk_pScript.data());
    if (!lk_Script_ || !lk_Script_->isGood())
        return NULL;
    return new k_ScriptBox(lk_pScript, ak_Parent_, ak_Proteomatic);
}


IDesktopBox*
k_DesktopBoxFactory::makeFileListBox(k_Desktop* ak_Parent_, 
                                      k_Proteomatic& ak_Proteomatic)
{
    return new k_FileListBox(ak_Parent_, ak_Proteomatic, NULL);
}


IDesktopBox*
k_DesktopBoxFactory::makeOutFileListBox(k_Desktop* ak_Parent_, 
                                        k_Proteomatic& ak_Proteomatic,
                                        QString as_Key,
                                        QString as_Label,
                                        IScriptBox* ak_ScriptBoxParent_)
{
    k_FileListBox* lk_Box_ = new k_FileListBox(ak_Parent_, ak_Proteomatic, ak_ScriptBoxParent_);
    lk_Box_->setLabel(as_Label);
    lk_Box_->setKey(as_Key);
    return lk_Box_;
}


IDesktopBox* 
k_DesktopBoxFactory::makeInputGroupProxyBox(k_Desktop* ak_Parent_, 
                                             k_Proteomatic& ak_Proteomatic,
                                             QString as_Label, QString as_GroupKey)
{
    return new k_InputGroupProxyBox(ak_Parent_, ak_Proteomatic, as_Label, as_GroupKey);
}

