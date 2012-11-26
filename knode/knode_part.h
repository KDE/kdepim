/*
    This file is part of KMail.
    Copyright (c) 2003      Laurent Montel <montel@kde.org>,
    Based on the work of Cornelius Schumacher <schumacher@kde.org>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#ifndef KNODE_PART_H
#define KNODE_PART_H


#include <kparts/browserextension.h>
#include <kparts/event.h>
#include <kparts/part.h>

#include <QWidget>

class KAboutData;

class ActionManager;
class KNMainWidget;

/** KNode part, used for embedding in Kontact. */
class KNodePart: public KParts::ReadOnlyPart
{
    Q_OBJECT
  public:
    KNodePart(QWidget *parentWidget, QObject *parent, const QVariantList &);
    virtual ~KNodePart();

    QWidget* parentWidget() const;

    static KAboutData *createAboutData();

  protected:
    virtual bool openFile();
    virtual void guiActivateEvent(KParts::GUIActivateEvent *e);

  private:
    QWidget *mParentWidget;
    KNMainWidget *mainWidget;
};

#endif
