/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "baloodebugsearchpathcombobox.h"
#include <KGlobal>
#include <KStandardDirs>
using namespace PimCommon;

BalooDebugSearchPathComboBox::BalooDebugSearchPathComboBox(QWidget *parent)
    : QComboBox(parent)
{
    initialize();
}

BalooDebugSearchPathComboBox::~BalooDebugSearchPathComboBox()
{

}

void BalooDebugSearchPathComboBox::initialize()
{
    const QString xdgpath = KGlobal::dirs()->localxdgdatadir();
    addItem(QLatin1String("Contacts"), QString(xdgpath + QLatin1String("baloo/contacts/")));
    addItem(QLatin1String("ContactCompleter"), QString(xdgpath + QLatin1String("baloo/emailContacts/")));
    addItem(QLatin1String("Email"), QString(xdgpath + QLatin1String("baloo/email/")));
    addItem(QLatin1String("Notes"), QString(xdgpath + QLatin1String("baloo/notes/")));
}
