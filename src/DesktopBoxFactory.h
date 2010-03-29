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

#include "IDesktopBox.h"
#include <QtCore>
#include <QtGui>
#include "RefPtr.h"


class k_Desktop;
class k_Proteomatic;


class k_DesktopBoxFactory
{
public:
    static IDesktopBox* makeScriptBox(QString as_ScriptUri, k_Desktop* ak_Parent_, k_Proteomatic& ak_Proteomatic);
    static IDesktopBox* makeFileListBox(k_Desktop* ak_Parent_, k_Proteomatic& ak_Proteomatic);
    static IDesktopBox* makeOutFileListBox(k_Desktop* ak_Parent_, k_Proteomatic& ak_Proteomatic, QString as_Key, QString as_Label);
    static IDesktopBox* makeInputGroupProxyBox(k_Desktop* ak_Parent_, k_Proteomatic& ak_Proteomatic, QString as_Label, QString as_GroupKey);
};
