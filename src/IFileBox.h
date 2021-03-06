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


struct IScriptBox;


struct IFileBox
{
    virtual ~IFileBox() {};
    
    virtual QStringList filenames() const = 0;
    virtual QString tagForFilename(const QString& as_Filename) const = 0;
    virtual QStringList filenamesForTag(const QString& as_Tag) const = 0;
    virtual QString prefixWithoutTags() const = 0;
    virtual void setListMode(bool ab_Enabled) = 0;
    virtual bool listMode() const = 0;
    virtual IScriptBox* scriptBoxParent() const = 0;
    
    // slots
    
    // signals
    virtual void arrowPressed() = 0;
    virtual void arrowReleased() = 0;
    virtual void filenamesChanged() = 0;
};
