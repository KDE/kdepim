/*  -*- c++ -*-
    keyselectiondialog.h

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2004 Klarälvdalens Datakonsult AB

    Based on kpgpui.h
    Copyright (C) 2001,2002 the KPGP authors
    See file libkdenetwork/AUTHORS.kpgp for details

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

#ifndef __KLEO_KEYAPPROVALDIALOG_H__
#define __KLEO_KEYAPPROVALDIALOG_H__

#include <kleo/enum.h>

#include <kdialogbase.h>
#include <kdepimmacros.h>

#include <kpgpkey.h> // for EncryptPref
#include <gpgmepp/key.h>

#include <vector>

namespace GpgME {
  class Key;
}

class QStringList;

namespace Kleo {

  class KDE_EXPORT KeyApprovalDialog : public KDialogBase {
    Q_OBJECT
  public:
    struct Item {
      Item() : pref( UnknownPreference ) {}
      Item( const QString & a, const std::vector<GpgME::Key> & k,
	    EncryptionPreference p=UnknownPreference )
	: address( a ), keys( k ), pref( p ) {}
      QString address;
      std::vector<GpgME::Key> keys;
      EncryptionPreference pref;
    };

    KeyApprovalDialog( const std::vector<Item> & recipients,
		       const std::vector<GpgME::Key> & sender,
		       QWidget * parent=0, const char * name=0,
                       bool modal=true );
    ~KeyApprovalDialog();

    std::vector<Item> items() const;
    std::vector<GpgME::Key> senderKeys() const;

    bool preferencesChanged() const;

  private slots:
    void slotPrefsChanged();

  private:
    class Private;
    Private * d;
  };

} // namespace Kleo

#endif // __KLEO_KEYAPPROVALDIALOG_H__
