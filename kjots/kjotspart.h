/*
    This file is part of KJots.

    Copyright (c) 2008 Stephen Kelly <steveire@gmail.com>

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

#ifndef KJOTSPART_H
#define KJOTSPART_H

#include <kparts/part.h>
#include <kparts/statusbarextension.h>
#include <kparts/readonlypart.h>

class QWidget;
class KAboutData;
class KJotsWidget;

/**
 * This is a "Part".  It that does all the real work in a KPart
 * application.
 *
 * @short Main Part
 * @author Stephen Kelly <steveire@gmail.com>
 * @version 0.1
 */
class KJotsPart : public KParts::ReadOnlyPart
{
    Q_OBJECT
public:
    /**
     * Default constructor
     */
    KJotsPart(QWidget *parentWidget,QObject *parent, const QVariantList &);

    /**
     * Destructor
     */
    virtual ~KJotsPart();

    static KAboutData *createAboutData();

protected:
    /**
     * This must be implemented by each part
     */
    virtual bool openFile();

protected Q_SLOTS:
    void delayedInitialization();
    void activeAnchorChanged(const QString &, const QString &);

private:
    void initAction();
    KJotsWidget *mComponent;
    KParts::StatusBarExtension *mStatusBar;
};

#endif // KJOTSPART_H
