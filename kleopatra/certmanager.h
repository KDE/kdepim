/* -*- mode: c++; c-basic-offset:4 -*-
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

#ifndef _CERTMANAGER_H_
#define _CERTMANAGER_H_

//#include <gpgme.h>

#include <kmainwindow.h>
#include <kdemacros.h>
#include <kurl.h>
#include <q3cstring.h>
#include <q3ptrlist.h>
//Added by qt3to4:
#include <QLabel>

#include <set>
#include <string>

namespace Kleo {
  class KeyListView;
  class KeyListViewItem;
  class ProgressBar;
  class Job;
}

class KJob;
namespace KIO {
  class Job;
}
class KProcess;
class KToolBar;
class KAction;

class CRLView;
class HierarchyAnalyser;

class LineEditAction;
class ComboAction;

class KUrl;
class QFile;
class QStringList;
class QLabel;

namespace GpgME {
  class ImportResult;
  class KeyListResult;
  class Error;
  class Key;
}

class KDE_EXPORT CertManager : public KMainWindow {
  Q_OBJECT
public:
    CertManager( bool remote = false, const QString& query = QString(),
		 const QString& import=QString(),
		 QWidget * parent=0, Qt::WFlags f=0 );
    ~CertManager();

    bool isRemote() const { return mRemote; }

signals:
    void stopOperations();
    void enableOperations( bool );

private slots:
    void slotStartCertificateDownload( const QString & fingerprint, const QString& displayName );
    void newCertificate();
    void revokeCertificate();
    void extendCertificate();
    void slotDeleteCertificate();
    void slotExportSecretKey();
    void slotExportCertificate();
    void slotUploadResult( KJob* job );

    void slotImportCertFromFile();
    void slotImportCertFromFile( const KUrl & filename );
    void slotImportResult( KJob* );

    void slotCertificateImportResult( const GpgME::ImportResult & result );
    void slotCertificateDownloadResult( const GpgME::Error & error, const QByteArray & keyData );
    void slotKeyListResult( const GpgME::KeyListResult & result );
    void slotDeleteResult( const GpgME::Error & error, const GpgME::Key & );
    void slotSecretKeyExportResult( const GpgME::Error & error, const QByteArray & keyData );
    void slotCertificateExportResult( const GpgME::Error & error, const QByteArray & keyData );
    void slotClearCRLsResult();

    void importCRLFromFile();
    void importCRLFromLDAP();
    void slotImportCRLJobFinished( KJob * );

    void slotDirmngrExited();
    void slotStderr( KProcess*, char*, int );

    void slotToggleRemote(int idx);
    void slotToggleHierarchicalView( bool );

    void slotViewCRLs();
    void slotClearCRLs();

    void slotViewDetails();
    void slotViewDetails( Kleo::KeyListViewItem * item );
    void slotSelectionChanged();
    void slotDownloadCertificate();
    void slotStartWatchGnuPG();

    void slotEditKeybindings();
    void slotShowConfigurationDialog();
    void slotConfigureGpgME();
    void slotContextMenu(Kleo::KeyListViewItem*, const QPoint& point);
    void slotDropped(const KUrl::List&);
    /** Schedule a repaint for the listview items. E.g. when the
	colour config has changed */
    void slotRepaint();
    /** Schedule a validating keylisting for the selected items (or
	all items, if none is selected). */
    void slotValidate() { startRedisplay( true ); }
    /** Schedule a non-validating keylisting for the selected items
	(or all items, if none are selected). */
    void slotRedisplay() { startRedisplay( false ); }
    /** Start a keylisting with the current value of the query text as
	pattern. */
    void slotSearch();

    void slotExpandAll();
    void slotCollapseAll();
    void slotRefreshKeys();
    void slotRefreshKeysResult( const GpgME::Error & );

private:
    void createStatusBar();
    void createActions();
    void updateStatusBarLabels();
    void updateImportActions( bool enable );
    void startKeyListing( bool, bool, const QStringList & );
    void startKeyListing( bool, bool, const std::set<std::string> & );
    void startCertificateImport( const QByteArray & keyData, const QString& certDisplayName );
    void startImportCRL( const QString& fileName, bool isTempFile );
    void startClearCRLs();
    void startSecretKeyExport( const QString & fingerprint );
    void startCertificateExport( const QStringList & fingerprints );
    bool connectAndStartDirmngr( const char*, const char* );
    void connectJobToStatusBarProgress( Kleo::Job * job, const QString & initialText );
    void disconnectJobFromStatusBarProgress( const GpgME::Error & err );
    void importNextURLOrRedisplay();
    void startRedisplay( bool validating );
    QString displayNameForJob( const Kleo::Job *job );
    void readConfig( bool noQueryGiven );
    void writeConfig();

private:
    Kleo::KeyListView * mKeyListView;
    CRLView * mCrlView;
    Kleo::ProgressBar * mProgressBar;
    QLabel * mStatusLabel;

    KProcess * mDirmngrProc;
    QString mErrorbuffer;
    Q3PtrList<Kleo::KeyListViewItem> mItemsToDelete;
    KUrl::List mURLsToImport;
    typedef QMap<const Kleo::Job *, QString> JobsDisplayNameMap;
    JobsDisplayNameMap mJobsDisplayNameMap;
    HierarchyAnalyser * mHierarchyAnalyser;

    LineEditAction * mLineEditAction;
    ComboAction * mComboAction;
    QAction * mFindAction;
    QAction * mImportCertFromFileAction;
    QAction * mImportCRLFromFileAction;
    QAction * mExportCertificateAction;
    QAction * mViewCertDetailsAction;
    QAction * mDeleteCertificateAction;
#ifdef NOT_IMPLEMENTED_ANYWAY
    KAction * mRevokeCertificateAction;
    KAction * mExtendCertificateAction;
#endif
    QAction * mExportSecretKeyAction;
    QAction * mDownloadCertificateAction;
    QAction * mValidateCertificateAction;

    QString mImportCRLTempFile;
    QString mCurrentQuery;
    std::set<std::string> mPreviouslySelectedFingerprints;
    bool     mNextFindRemote : 1; // state of the combo, i.e. whether the next find action will be remote
    bool     mRemote : 1; // whether the currently displayed items are from a remote listing
    bool     mDirMngrFound : 1;
    bool     mHierarchicalView : 1; // whether to display the list view in hierarchical mode

};

#endif // _CERTMANAGER_H_
