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

#include "ScriptFactory.h"
#include "IScript.h"
#include "Script.h"
#include "LocalScript.h"
#include "RemoteScript.h"


RefPtr<IScript> k_ScriptFactory::makeScript(QString as_ScriptUri, k_Proteomatic& ak_Proteomatic, bool ab_IncludeOutputFiles, bool ab_ProfileMode)
{
    k_Script* lk_Script_ = NULL;
    if (as_ScriptUri.startsWith("druby://"))
        //lk_Script_ = new k_RemoteScript(as_ScriptUri, ak_Proteomatic, ab_IncludeOutputFiles, ab_ProfileMode);
        lk_Script_ = NULL;
    else
        lk_Script_ = new k_LocalScript(as_ScriptUri, ak_Proteomatic, ab_IncludeOutputFiles, ab_ProfileMode);
    
    if (lk_Script_ && (!lk_Script_->isGood()))
    {
        delete lk_Script_;
        lk_Script_ = NULL;
    }
        
    return RefPtr<IScript>(lk_Script_);
}
