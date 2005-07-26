/*
    appearanceconfigwidget.h

    This file is part of kleopatra, the KDE key manager
    Copyright (c) 2002,2004 Klar�lvdalens Datakonsult AB
    Copyright (c) 2002,2003 Marc Mutz <mutz@kde.org>

    Libkleopatra is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.

    Libkleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
 */

#ifndef APPEARANCECONFIGWIDGET_H
#define APPEARANCECONFIGWIDGET_H

#include "appearanceconfigwidgetbase.h"
#include <qstringlist.h>
class KConfig;

namespace Kleo {

  class AppearanceConfigWidget : public AppearanceConfigWidgetBase {
    Q_OBJECT

  public:
    AppearanceConfigWidget(
      QWidget * parent=0, const char * name=0, WFlags f=0 );
    ~AppearanceConfigWidget();

    void load();
    void save();

  public slots:
    void defaults();

  signals:
    void changed();

  protected slots:
    // reimplemented from the base class
    virtual void slotDefaultClicked();
    virtual void slotSelectionChanged( QListViewItem * );
    virtual void slotForegroundClicked();
    virtual void slotBackgroundClicked();
    virtual void slotFontClicked();
    virtual void slotItalicClicked();
    virtual void slotBoldClicked();
    virtual void slotStrikeoutClicked();
  };
}

#endif // APPEARANCECONFIGWIDGET_H
