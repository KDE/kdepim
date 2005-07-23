/*  -*- c++ -*-
    passphrasedialog.h

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


    Based on kpgpui.h
    Copyright (C) 2001,2002 the KPGP authors
    See file libkdenetwork/AUTHORS.kpgp for details

    This file is part of KPGP, the KDE PGP/GnuPG support library.

    KPGP is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
*/

#ifndef __KLEO_UI_PASSPHRASEDIALOG_H__
#define __KLEO_UI_PASSPHRASEDIALOG_H__

#include <kdialogbase.h>

namespace Kleo {

  class PassphraseDialog : public KDialogBase {
    Q_OBJECT
  public:
    PassphraseDialog( const QString & description,
		      const QString & caption=QString::null,
		      QWidget * parent=0, const char * name=0,
                      bool modal=true );
    ~PassphraseDialog();

    const char * passphrase() const;

  signals:
    /** emitted when the user clicks Ok. \a pass is never NULL.
	\c pass only valid inside slots connected to this signal.
     */
    void finished( const char * pass );
    /** emitted when the user clicks Cancel. */
    void canceled();

  protected slots:
    /*! \reimp */
    void slotOk();
    /*! \reimp */
    void slotCancel();

  private:
    class Private;
    Private * d;
  protected:
    /*! \reimp */
    void virtual_hook( int, void* );
  };

} // namespace Kleo

#endif // __KLEO_UI_PASSPHRASEDIALOG_H__
