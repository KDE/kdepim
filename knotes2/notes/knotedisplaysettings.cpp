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
}

QColor KNoteDisplaySettings::backgroundColor() const
{
    if (mDisplayAttribute)
        return mDisplayAttribute->backgroundColor();
    else
        return QColor(); //TODO
}

QColor KNoteDisplaySettings::foregroundColor() const
{
    if (mDisplayAttribute)
        return mDisplayAttribute->foregroundColor();
    else
        return QColor(); //TODO
}

QSize KNoteDisplaySettings::size() const
{
    if (mDisplayAttribute)
        return mDisplayAttribute->size();
    else
        return QSize();
}

bool KNoteDisplaySettings::rememberDesktop() const
{
    if (mDisplayAttribute)
        return mDisplayAttribute->rememberDesktop();
    else
        return false;// TODO
}

int KNoteDisplaySettings::tabSize() const
{
    if (mDisplayAttribute)
        return mDisplayAttribute->tabSize();
    else
        return 1;// TODO
}

QFont KNoteDisplaySettings::font() const
{
    if (mDisplayAttribute)
        return mDisplayAttribute->font();
    else
        return QFont();// TODO
}

QFont KNoteDisplaySettings::titleFont() const
{
    if (mDisplayAttribute)
        return mDisplayAttribute->titleFont();
    else
        return QFont();// TODO
}

int KNoteDisplaySettings::desktop() const
{
    if (mDisplayAttribute)
        return mDisplayAttribute->desktop();
    else
        return 1;// TODO
}

bool KNoteDisplaySettings::isHidden() const
{
    if (mDisplayAttribute)
        return mDisplayAttribute->isHidden();
    else
        return false;// TODO
}

QPoint KNoteDisplaySettings::position() const
{
    if (mDisplayAttribute)
        return mDisplayAttribute->position();
    else
        return QPoint();// TODO
}

bool KNoteDisplaySettings::showInTaskbar() const
{
    if (mDisplayAttribute)
        return mDisplayAttribute->showInTaskbar();
    else
        return false;// TODO
}

bool KNoteDisplaySettings::keepAbove() const
{
    if (mDisplayAttribute)
        return mDisplayAttribute->keepAbove();
    else
        return false;// TODO
}

bool KNoteDisplaySettings::keepBelow() const
{
    if (mDisplayAttribute)
        return mDisplayAttribute->keepBelow();
    else
        return false;// TODO
}

bool KNoteDisplaySettings::autoIndent() const
{
    if (mDisplayAttribute)
        return mDisplayAttribute->autoIndent();
    else
        return false;// TODO
}
