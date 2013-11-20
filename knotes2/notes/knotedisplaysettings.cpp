/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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


#include "knotedisplaysettings.h"
#include "noteshared/attributes/notedisplayattribute.h"

#include <KGlobalSettings>
#include <QDebug>

KNoteDisplaySettings::KNoteDisplaySettings(NoteShared::NoteDisplayAttribute *attr)
    : mDisplayAttribute(attr)
{
}

KNoteDisplaySettings::~KNoteDisplaySettings()
{

}

void KNoteDisplaySettings::setDisplayAttribute(NoteShared::NoteDisplayAttribute *attr)
{
    mDisplayAttribute = attr;
    qDebug()<<" set !mDisplayAttribute->position()"<<mDisplayAttribute->position();
}

QColor KNoteDisplaySettings::backgroundColor() const
{
    if (mDisplayAttribute)
        return mDisplayAttribute->backgroundColor();
    else
        return QColor(Qt::yellow);
}

QColor KNoteDisplaySettings::foregroundColor() const
{
    if (mDisplayAttribute)
        return mDisplayAttribute->foregroundColor();
    else
        return QColor(Qt::black);
}

QSize KNoteDisplaySettings::size() const
{
    if (mDisplayAttribute)
        return mDisplayAttribute->size();
    else
        return QSize(300,300);
}

bool KNoteDisplaySettings::rememberDesktop() const
{
    if (mDisplayAttribute)
        return mDisplayAttribute->rememberDesktop();
    else
        return true;
}

int KNoteDisplaySettings::tabSize() const
{
    if (mDisplayAttribute)
        return mDisplayAttribute->tabSize();
    else
        return 4;
}

QFont KNoteDisplaySettings::font() const
{
    if (mDisplayAttribute)
        return mDisplayAttribute->font();
    else
        return KGlobalSettings::generalFont();
}

QFont KNoteDisplaySettings::titleFont() const
{
    if (mDisplayAttribute)
        return mDisplayAttribute->titleFont();
    else
        return KGlobalSettings::windowTitleFont();
}

int KNoteDisplaySettings::desktop() const
{
    if (mDisplayAttribute)
        return mDisplayAttribute->desktop();
    else
        return -10;
}

bool KNoteDisplaySettings::isHidden() const
{
    if (mDisplayAttribute)
        return mDisplayAttribute->isHidden();
    else
        return false;
}

QPoint KNoteDisplaySettings::position() const
{
    if (mDisplayAttribute) {
        qDebug()<<" mDisplayAttribute->position()"<<mDisplayAttribute->position();
        return mDisplayAttribute->position();
    }
    else
        return QPoint( -10000, -10000 );
}

bool KNoteDisplaySettings::showInTaskbar() const
{
    if (mDisplayAttribute)
        return mDisplayAttribute->showInTaskbar();
    else
        return false;
}

bool KNoteDisplaySettings::keepAbove() const
{
    if (mDisplayAttribute)
        return mDisplayAttribute->keepAbove();
    else
        return false;
}

bool KNoteDisplaySettings::keepBelow() const
{
    if (mDisplayAttribute)
        return mDisplayAttribute->keepBelow();
    else
        return false;
}

bool KNoteDisplaySettings::autoIndent() const
{
    if (mDisplayAttribute)
        return mDisplayAttribute->autoIndent();
    else
        return true;
}
