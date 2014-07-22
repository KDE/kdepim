/*
    cryptoconfigmodule.h

    This file is part of libkleopatra
    Copyright (c) 2004,2005 Klarälvdalens Datakonsult AB

    Libkleopatra is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License,
    version 2, as published by the Free Software Foundation.

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

#ifndef CRYPTOCONFIGMODULE_H
#define CRYPTOCONFIGMODULE_H

#include "kleo_export.h"
#include <kpagedialog.h>
#include <QtCore/QList>

namespace Kleo {

  class CryptoConfig;
  class CryptoConfigComponentGUI;

  struct ParsedKeyserver {
      QString url;
      QVector< QPair<QString,QString> > options;
  };

  KLEO_EXPORT ParsedKeyserver parseKeyserver( const QString & str );
  KLEO_EXPORT QString assembleKeyserver( const ParsedKeyserver & keyserver );


  /**
   * Crypto Config Module widget, dynamically generated from CryptoConfig
   * It's a simple QWidget so that it can be embedded into a dialog or into a KCModule.
   */
  class KLEO_EXPORT CryptoConfigModule : public KPageWidget {
    Q_OBJECT
  public:
    enum Layout { TabbedLayout, IconListLayout, LinearizedLayout };
    explicit CryptoConfigModule( Kleo::CryptoConfig* config, QWidget * parent=0 );
    explicit CryptoConfigModule( Kleo::CryptoConfig* config, Layout layout, QWidget * parent=0 );

    bool hasError() const;

    void save();
    void reset(); // i.e. reload current settings, discarding user input
    void defaults();
    void cancel();

  Q_SIGNALS:
    void changed();

  private:
    void init( Layout layout );

  private:
    Kleo::CryptoConfig* mConfig;
    QList<CryptoConfigComponentGUI *> mComponentGUIs;
  };

}

#endif
