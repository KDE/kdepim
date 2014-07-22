/*  -*- c++ -*-
    dnattributeorderconfigwidget.h

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2004 Klarälvdalens Datakonsult AB

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

#ifndef __KLEO_UI_DNATTRIBUTEORDERCONFIGWIDGET_H__
#define __KLEO_UI_DNATTRIBUTEORDERCONFIGWIDGET_H__

#include "kleo_export.h"

#include <QWidget>

namespace Kleo {
  class DNAttributeMapper;
}

class QTreeWidgetItem;

namespace Kleo {

  class KLEO_EXPORT DNAttributeOrderConfigWidget : public QWidget {
    Q_OBJECT
  public:
    /*! Use Kleo::DNAttributeMapper::instance()->configWidget( parent, name ) instead. */
    explicit DNAttributeOrderConfigWidget( DNAttributeMapper * mapper, QWidget * parent=0, Qt::WindowFlags f=0 );
    ~DNAttributeOrderConfigWidget();

    void load();
    void save() const;
    void defaults();

  Q_SIGNALS:
    void changed();

    //
    // only boring stuff below...
    //

  private Q_SLOTS:
    void slotAvailableSelectionChanged( QTreeWidgetItem * );
    void slotCurrentOrderSelectionChanged( QTreeWidgetItem * );
    void slotDoubleUpButtonClicked();
    void slotUpButtonClicked();
    void slotDownButtonClicked();
    void slotDoubleDownButtonClicked();
    void slotLeftButtonClicked();
    void slotRightButtonClicked();

  private:
    void takePlaceHolderItem();
    void enableDisableButtons( QTreeWidgetItem * );

  private:
    class Private;
    Private *const d;
  protected:
    virtual void virtual_hook( int, void* );
  };

}

#endif // __KLEO_UI_DNATTRIBUTEORDERCONFIGWIDGET_H__
