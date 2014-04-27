/*
    This file is part of Akonadi.

    Copyright (c) 2011 Stephen Kelly <steveire@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
    USA.
*/

#ifndef COISCEIMPART_H
#define COISCEIMPART_H

#include <kparts/part.h>
#include <kparts/statusbarextension.h>
#include <kparts/readonlypart.h>
#include "coisceimwidget.h"

class QWidget;
class KAboutData;

class CoisceimPart : public KParts::ReadOnlyPart
{
    Q_OBJECT
public:
    /**
     * Default constructor
     */
    CoisceimPart(QWidget *parentWidget,QObject *parent, const QVariantList &);

    /**
     * Destructor
     */
    virtual ~CoisceimPart();

    static KAboutData *createAboutData();

private:
    CoisceimWidget *component;
};

#endif // COISCEIMPART_H
