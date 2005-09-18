/*
    certificateinfowidgetimpl.h

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2001,2004 Klarälvdalens Datakonsult AB

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

#ifndef CERTIFICATEINFOWIDGETIMPL_H
#define CERTIFICATEINFOWIDGETIMPL_H

#include "certificateinfowidget.h"

#include <gpgmepp/key.h>

#include <q3valuelist.h>
//Added by qt3to4:
#include <QByteArray>
#include <QList>

class KProcess;
class Q3ListViewItem;

namespace GpgME {
  class KeyListResult;
}

class CertificateInfoWidgetImpl : public CertificateInfoWidget {
  Q_OBJECT
public:
  CertificateInfoWidgetImpl( const GpgME::Key & key, bool external,
			     QWidget * parent=0, const char * name=0);

  void setKey( const GpgME::Key & key );

signals:
  void requestCertificateDownload( const QString & fingerprint, const QString& displayName );

private slots:
  void slotShowInfo( Q3ListViewItem* );
  void slotShowCertPathDetails( Q3ListViewItem* );
  void slotImportCertificate();
  void slotCertificateChainListingResult( const GpgME::KeyListResult & res );
  void slotNextKey( const GpgME::Key & key );
  void slotKeyExistanceCheckNextCandidate( const GpgME::Key & key );
  void slotKeyExistanceCheckFinished();
  void slotCollectStdout(KProcess *, char *, int);
  void slotCollectStderr(KProcess *, char *, int);
  void slotDumpProcessExited(KProcess*);

private:
  void startCertificateChainListing();
  void startCertificateDump();
  void startKeyExistanceCheck();
  void updateChainView();

private:
  QByteArray mDumpOutput;
  QByteArray mDumpError;
  QList<GpgME::Key> mChain;
  bool mExternal;
  bool mFoundIssuer;
  bool mHaveKeyLocally;
};

#endif // CERTIFICATEINFOWIDGETIMPL_H
