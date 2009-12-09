/*
Copyright (c) 2009 Michael Specht

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


#include "NoSlashValidator.h"


k_NoSlashValidator::k_NoSlashValidator(QObject* ak_Parent_)
    : QValidator(ak_Parent_)
{
}


k_NoSlashValidator::~k_NoSlashValidator()
{
}


void k_NoSlashValidator::fixup(QString& as_String) const
{
    as_String.replace("/", "");
    as_String.replace("\\", "");
}


QValidator::State k_NoSlashValidator::validate(QString& as_String, int& li_Position) const
{
    if (as_String.contains("/"))
    {
        li_Position = as_String.indexOf("/");
        return QValidator::Invalid;
    }
    if (as_String.contains("\\"))
    {
        li_Position = as_String.indexOf("\\");
        return QValidator::Invalid;
    }
    return QValidator::Acceptable;
}
