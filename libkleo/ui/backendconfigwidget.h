/*  -*- c++ -*-
    backendconfigwidget.h

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2002,2004 Klarälvdalens Datakonsult AB
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

#ifndef __KLEO_UI_BACKENDCONFIGWIDGET_H__
#define __KLEO_UI_BACKENDCONFIGWIDGET_H__

#include "kleo/kleo_export.h"
#include <QtGui/QWidget>

namespace Kleo {
  class CryptoBackendFactory;
}

class Q3ListViewItem;

namespace Kleo {

  class KLEO_EXPORT BackendConfigWidget : public QWidget {
    Q_OBJECT
  public:
    explicit BackendConfigWidget( CryptoBackendFactory * factory, QWidget * parent=0, const char * name=0, Qt::WFlags f=0 );
    ~BackendConfigWidget();

    void load();
    void save() const;

    void emitChanged( bool b ) { changed( b ); }

  Q_SIGNALS:
    void changed( bool );

  private Q_SLOTS:
    void slotSelectionChanged( Q3ListViewItem * );
    void slotRescanButtonClicked();
    void slotConfigureButtonClicked();

  private:
    class Private;
    Private * d;
  protected:
    virtual void virtual_hook( int, void* );
  };

}

#endif // __KLEO_UI_BACKENDCONFIGWIDGET_H__
