/* -*- mode: c++; c-basic-offset:4 -*-
    certlistview.h

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2007 Klarälvdalens Datakonsult AB

    Kleopatra is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Kleopatra is distributed in the hope that it will be useful,
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

#ifndef CERTLISTVIEW_H
#define CERTLISTVIEW_H

#include "libkleo/ui/keylistview.h"
#include <kurl.h>
//Added by qt3to4:
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QDropEvent>

/// We need to derive from Kleo::KeyListView simply to add support for drop events
class CertKeyListView : public Kleo::KeyListView {
  Q_OBJECT

public:
  CertKeyListView( const ColumnStrategy * strategy,
                   const DisplayStrategy * display=0,
                   QWidget * parent=0, Qt::WFlags f=0 );

signals:
  void dropped( const KUrl::List& urls );

protected:
  virtual void contentsDragEnterEvent ( QDragEnterEvent * );
  virtual void contentsDragMoveEvent( QDragMoveEvent * );
  virtual void contentsDragLeaveEvent( QDragLeaveEvent * );
  virtual void contentsDropEvent ( QDropEvent * );

};


#endif /* CERTLISTVIEW_H */
