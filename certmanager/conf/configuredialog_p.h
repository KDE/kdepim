/*
    configuredialog_p.h

    This file is part of kleopatra
    Copyright (C) 2000 Espen Sand, espen@kde.org
    Copyright (C) 2001-2002 Marc Mutz <mutz@kde.org>
    Copyright (c) 2004 Klarälvdalens Datakonsult AB

    Libkleopatra is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License,
    version 2, as published by the Free Software Foundation.

    Libkleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

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

// configuredialog_p.h: classes internal to ConfigureDialog
// see configuredialog.h for details.

#ifndef _CONFIGURE_DIALOG_PRIVATE_H_
#define _CONFIGURE_DIALOG_PRIVATE_H_

#include <kcmodule.h>

namespace Kleo {
  class CryptoConfig;
  class CryptoConfigEntry;
  class DirectoryServicesWidget;
}

/**
 * DirectoryServicesConfigurationPage
 */
class DirectoryServicesConfigurationPage : public KCModule {
  Q_OBJECT
public:
  DirectoryServicesConfigurationPage( QWidget * parent=0, const char * name=0 );

  virtual void load();
  virtual void save();
  virtual void defaults();

private slots:
  void slotChanged();

private:
  Kleo::CryptoConfigEntry* configEntry();

  Kleo::DirectoryServicesWidget* mWidget;
  Kleo::CryptoConfig* mConfig;
};

#endif // _CONFIGURE_DIALOG_PRIVATE_H_
