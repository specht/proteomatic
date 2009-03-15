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
#include "DesktopBox.h"


class k_Proteomatic;

class k_ScriptBox: public k_DesktopBox, public IScriptBox
{
	Q_OBJECT
public:
	k_ScriptBox(const QString& as_ScriptUri, k_Desktop* ak_Parent_, k_Proteomatic& ak_Proteomatic);
	virtual ~k_ScriptBox();
	
	virtual IScript* script();
	
protected:
	virtual void setupLayout();
	
	k_Proteomatic& mk_Proteomatic;
	RefPtr<IScript> mk_pScript;
	RefPtr<QWidget> mk_pParameterProxyWidget;
};
