/*  -*- mode: C++; c-file-style: "gnu" -*-
    certmanager.h

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2001,2002,2004 Klarälvdalens Datakonsult AB

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

#ifndef _CERTMANAGER_H_
#define _CERTMANAGER_H_

//#include <gpgme.h>

#include <kmainwindow.h>

#include <qstring.h>
#include <qcstring.h>
#include <qptrlist.h>

namespace Kleo {
  class KeyListView;
  class KeyListViewItem;
  class ProgressBar;
}

namespace KIO {
  class Job;
}
class KProcess;
class KToolBar;
class KAction;

class CRLView;

class LineEditAction;
class ComboAction;

class KURL;
class QFile;
class QStringList;
class QLabel;

namespace GpgME {
  class ImportResult;
  class KeyListResult;
  class Error;
  class Key;
}

class CertManager : public KMainWindow {
  Q_OBJECT
public:
    CertManager( bool remote = false, const QString& query = QString::null,
		 const QString& import=QString::null,
		 QWidget* parent = 0, const char* name = 0);

    bool isRemote() const { return mRemote; }

private slots:
    void slotStartCertificateDownload( const QString & fingerprint );
    void slotStartCertificateListing();
    void newCertificate();
    void quit();
    void revokeCertificate();
    void extendCertificate();
    void slotDeleteCertificate();
    void slotExportSecretKey();
    void slotExportCertificate();
    void slotUploadResult( KIO::Job* job );

    void slotImportCertFromFile();
    void slotImportCertFromFile( const KURL & filename );
    void slotImportResult( KIO::Job* );

    void slotCertificateImportResult( const GpgME::ImportResult & result );
    void slotCertificateDownloadResult( const GpgME::Error & error, const QByteArray & keyData );
    void slotKeyListResult( const GpgME::KeyListResult & result );
    void slotDeleteResult( const GpgME::Error & error, const GpgME::Key & );
    void slotSecretKeyExportResult( const GpgME::Error & error, const QByteArray & keyData );
    void slotCertificateExportResult( const GpgME::Error & error, const QByteArray & keyData );

    void importCRLFromFile();
    void importCRLFromLDAP();
    void slotImportCRLJobFinished( KIO::Job * );

    void slotDirmngrExited();
    void slotStderr( KProcess*, char*, int );

    void slotToggleRemote(int idx);

    void slotViewCRLs();

    void slotListViewItemActivated( Kleo::KeyListViewItem * item );

    void slotEditKeybindings();
    void slotShowConfigurationDialog();

private:
    void createStatusBar();
    void createActions();
    void updateStatusBarLabels();
    void updateImportActions( bool enable );
    void startCertificateImport( const QByteArray & keyData );
    void startImportCRL( const QString& fileName, bool isTempFile );
    void startSecretKeyExport( const QString & fingerprint );
    void startCertificateExport( const QStringList & fingerprints );

private:
    Kleo::KeyListView * mKeyListView;
    CRLView * mCrlView;
    Kleo::ProgressBar * mProgressBar;
    QLabel * mStatusLabel;

    KProcess * mDirmngrProc;
    QString mErrorbuffer;
    QPtrList<Kleo::KeyListViewItem> mItemsToDelete;

    LineEditAction * mLineEditAction;
    ComboAction * mComboAction;
    KAction * mFindAction;
    KAction * mImportCertFromFileAction;
    KAction * mImportCRLFromFileAction;

    QString mImportCRLTempFile;
    bool     mRemote;
    bool     mDirMngrFound;
};

#endif // _CERTMANAGER_H_
