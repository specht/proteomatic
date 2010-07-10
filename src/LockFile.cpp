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

#include "LockFile.h"


k_LockFile::k_LockFile(const QString& as_Path)
    : mk_File(as_Path)
{
    this->touch();
}


k_LockFile::~k_LockFile()
{
    mk_File.remove();
}


void k_LockFile::touch()
{
    mk_File.open(QIODevice::WriteOnly);
    mk_File.close();
}
