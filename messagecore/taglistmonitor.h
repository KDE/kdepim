/* Copyright 2010 Casey Link <unnamedrambler@gmail.com>
   Copyright (C) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef MESSAGECORE_TAGLISTMONITOR_H
#define MESSAGECORE_TAGLISTMONITOR_H

#include "messagecore_export.h"

#include <QtCore/QObject>

namespace MessageCore {

/**
 * @short A class that acts as central notifier for changes to tags.
 *
 * @author Casey Link <unnamedrambler@gmail.com>
 */
class MESSAGECORE_EXPORT TagListMonitor: public QObject
{
  Q_OBJECT

  public:
    /**
     * Creates a new tag list monitor.
     *
     * @param parent The parent object.
     */
    TagListMonitor( QObject *parent = 0 );

    /**
     * Destroys the tag list monitor.
     */
    ~TagListMonitor();

    /**
     * This will emit the tagsChanged() on all TagListMonitor instances.
     */
    static void triggerUpdate();

  Q_SIGNALS:
    /**
     * This signal is emitted when the triggerUpdate() method has been called.
     */
    void tagsChanged();

  private:
    //@cond PRIVATE
    class Private;
    Private* const d;
    //@endcond
};

}

#endif
