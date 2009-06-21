/*
Copyright (c) 2007-2008 Thaddäus Slawicki

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

#include "MD5Thread.h"

MD5Thread::MD5Thread()
{
li_qmin=0;
li_qmax=0;
}

void MD5Thread::setValue(int li_min,int li_max)
{
li_qmin = li_min;
li_qmax = li_max;
}

void MD5Thread::run()
{
for (int i = 0; i < li_qmax; i++)
	{
	for (volatile int j = 0; j < 12345; j++);
	}
}