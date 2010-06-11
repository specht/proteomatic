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

#include "Preview.h"

k_Preview::k_Preview(const QString& as_Url, QWidget* parent, Qt::WindowFlags f)
    : QWidget(parent, f)
    , ms_Url(as_Url)
{
    this->setupLayout();
    mk_WebView.load(QUrl::fromLocalFile(ms_Url));
}


k_Preview::~k_Preview()
{
}


void k_Preview::setupLayout()
{
    QBoxLayout* lk_VLayout_ = new QVBoxLayout(this);
    lk_VLayout_->addWidget(new QLabel("<b>" + ms_Url + "</b>", this));
    lk_VLayout_->addWidget(&mk_WebView);
}
