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
#include "ScriptBox.h"
#include "FileListBox.h"


RefPtr<IDesktopBox> 
k_DesktopBoxFactory::makeScriptBox(QString as_ScriptUri, k_Desktop* ak_Parent_, 
								   k_Proteomatic& ak_Proteomatic)
{
	return RefPtr<IDesktopBox>(new k_ScriptBox(as_ScriptUri, ak_Parent_, ak_Proteomatic));
}


RefPtr<IDesktopBox> 
k_DesktopBoxFactory::makeFileListBox(k_Desktop* ak_Parent_, k_Proteomatic& ak_Proteomatic)
{
	return RefPtr<IDesktopBox>(new k_FileListBox(ak_Parent_, ak_Proteomatic));
}
