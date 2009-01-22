/* -*- mode: c++; c-basic-offset:4 -*-
    commands/importcertificatescommand.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2007, 2008 Klarälvdalens Datakonsult AB

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

#include <config-kleopatra.h>

#include "importcertificatescommand.h"
#include "importcertificatescommand_p.h"

#include <models/keylistsortfilterproxymodel.h>
#include <models/predicates.h>

#include <utils/formatting.h>

#include <kleo/cryptobackendfactory.h>
#include <kleo/importjob.h>

#include <gpgme++/global.h>
#include <gpgme++/importresult.h>

#include <KLocale>
#include <KMessageBox>
#include <KConfigGroup>
#include <KDebug>

#include <QByteArray>
#include <QFile>
#include <QFileDialog>
#include <QPointer>
#include <QString>
#include <QWidget>
#include <QFileInfo>
#include <QTreeView>

#include <memory>
#include <algorithm>
#include <cassert>

using namespace GpgME;
using namespace Kleo;

namespace {

    make_comparator_str( ByImportFingerprint, .fingerprint() );

    class ImportResultProxyModel : public AbstractKeyListSortFilterProxyModel {
        Q_OBJECT
    public:
        explicit ImportResultProxyModel( const ImportResult & result, QObject * parent=0 )
            : AbstractKeyListSortFilterProxyModel( parent ),
              m_result()
        {
            updateFindCache( result );
        }

        ~ImportResultProxyModel() {}

        /* reimp */ ImportResultProxyModel * clone() const {
            // compiler-generated copy ctor is fine!
            return new ImportResultProxyModel( *this );
        }

        void setImportResult( const ImportResult & result ) {
            m_result = result;
            updateFindCache( result );
            invalidateFilter();
        }

    protected:
        /* reimp */ QVariant data( const QModelIndex & index, int role ) const {
            if ( !index.isValid() || role != Qt::ToolTipRole )
                return AbstractKeyListSortFilterProxyModel::data( index, role );
            // get the fingerprint:
            const QModelIndex fprIndex = index.sibling( index.row(), AbstractKeyListModel::Fingerprint );
            assert( fprIndex.isValid() );
            const QString fpr = fprIndex.data( Qt::EditRole ).toString();
            // find information:
            const std::vector<Import>::const_iterator it
                = qBinaryFind( m_importsByFingerprint.begin(), m_importsByFingerprint.end(),
                               fpr.toLatin1().constData(),
                               ByImportFingerprint<std::less>() );
            if ( it == m_importsByFingerprint.end() )
                return AbstractKeyListSortFilterProxyModel::data( index, role );
            else
                return Formatting::importMetaData( *it );
        }
        /* reimp */ bool filterAcceptsRow( int source_row, const QModelIndex & source_parent ) const {
            //
            // 0. Keep parents of matching children:
            //
            const QModelIndex index = sourceModel()->index( source_row, 0, source_parent );
            assert( index.isValid() );
            for ( int i = 0, end = sourceModel()->rowCount( index ) ; i != end ; ++i )
                if ( filterAcceptsRow( i, index ) )
                    return true;
            //
            // 1. Check that this is an imported key:
            //
            const QModelIndex fprIndex = sourceModel()->index( source_row, AbstractKeyListModel::Fingerprint, source_parent );
            assert( fprIndex.isValid() );
            const QString fpr = fprIndex.data( Qt::EditRole ).toString();
            
            return std::binary_search( m_importsByFingerprint.begin(), m_importsByFingerprint.end(),
                                       fpr.toLatin1().constData(),
                                       ByImportFingerprint<std::less>() );
        }

    private:
        void updateFindCache( const ImportResult & result ) {
            m_importsByFingerprint.clear();
            m_result = result;
            m_importsByFingerprint = result.imports();
            std::sort( m_importsByFingerprint.begin(), m_importsByFingerprint.end(),
                       ByImportFingerprint<std::less>() );
        }

    private:
        mutable std::vector<Import> m_importsByFingerprint;
        ImportResult m_result;
    };

}

ImportCertificatesCommand::Private::Private( ImportCertificatesCommand * qq, KeyListController * c )
    : Command::Private( qq, c ), cmsImportJob( 0 ), pgpImportJob( 0 )
{
    
}

ImportCertificatesCommand::Private::~Private() {}

#define d d_func()
#define q q_func()


ImportCertificatesCommand::ImportCertificatesCommand( KeyListController * p )
    : Command( new Private( this, p ) )
{
    
}

ImportCertificatesCommand::ImportCertificatesCommand( QAbstractItemView * v, KeyListController * p )
    : Command( v, new Private( this, p ) )
{
    
}

ImportCertificatesCommand::~ImportCertificatesCommand() {}

void ImportCertificatesCommand::Private::setImportResultProxyModel( const ImportResult & result, const QString & id ) {
    if ( result.imports().empty() )
        return;
    q->addTemporaryView( i18n("Imported Certificates"), new ImportResultProxyModel( result ),
                         id.isEmpty() ? QString() : i18n( "Imported Certificates from %1", id ) );
    if ( QTreeView * const tv = qobject_cast<QTreeView*>( parentWidgetOrView() ) )
        tv->expandAll();
}

void ImportCertificatesCommand::Private::showDetails( QWidget * parent, const ImportResult & res, const QString & id ) {

    setImportResultProxyModel( res, id );

    const KLocalizedString normalLine = ki18n("<tr><td align=\"right\">%1</td><td>%2</td></tr>");
    const KLocalizedString boldLine = ki18n("<tr><td align=\"right\"><b>%1</b></td><td>%2</td></tr>");

    QStringList lines;
    lines.push_back( normalLine.subs( i18n("Total number processed:") )
                     .subs( res.numConsidered() ).toString() );
    lines.push_back( normalLine.subs( i18n("Imported:") )
                     .subs( res.numImported() ).toString() );
    if ( res.newSignatures() )
        lines.push_back( normalLine.subs( i18n("New signatures:") )
                         .subs( res.newSignatures() ).toString() );
    if ( res.newUserIDs() )
        lines.push_back( normalLine.subs( i18n("New user IDs:") )
                         .subs( res.newUserIDs() ).toString() );
    if ( res.numKeysWithoutUserID() )
        lines.push_back( normalLine.subs( i18n("Certificates without user IDs:") )
                         .subs( res.numKeysWithoutUserID() ).toString() );
    if ( res.newSubkeys() )
        lines.push_back( normalLine.subs( i18n("New subkeys:") )
                         .subs( res.newSubkeys() ).toString() );
    if ( res.newRevocations() )
        lines.push_back( boldLine.subs( i18n("Newly revoked:") )
                         .subs( res.newRevocations() ).toString() );
    if ( res.notImported() )
        lines.push_back( boldLine.subs( i18n("Not imported:") )
                         .subs( res.notImported() ).toString() );
    if ( res.numUnchanged() )
        lines.push_back( normalLine.subs( i18n("Unchanged:") )
                         .subs( res.numUnchanged() ).toString() );
    if ( res.numSecretKeysConsidered() )
        lines.push_back( normalLine.subs( i18n("Secret keys processed:") )
                         .subs( res.numSecretKeysConsidered() ).toString() );
    if ( res.numSecretKeysImported() )
        lines.push_back( normalLine.subs( i18n("Secret keys imported:") )
                         .subs( res.numSecretKeysImported() ).toString() );
    if ( res.numSecretKeysConsidered() - res.numSecretKeysImported() - res.numSecretKeysUnchanged() > 0 )
        lines.push_back( boldLine.subs( i18n("Secret keys <em>not</em> imported:") )
                         .subs(  res.numSecretKeysConsidered()
                                 - res.numSecretKeysImported()
                                 - res.numSecretKeysUnchanged() ).toString() );
    if ( res.numSecretKeysUnchanged() )
        lines.push_back( normalLine.subs( i18n("Secret keys unchanged:") )
                         .subs( res.numSecretKeysUnchanged() ).toString() );
    
    KMessageBox::information( parent,
                              id.isEmpty()
                              ? i18n( "<qt><p>Detailed results of certificate import:</p>"
                                      "<table>%1</table></qt>",
                                      lines.join( QString() ) )
                              : i18n( "<qt><p>Detailed results of importing %1:</p>"
                                      "<table>%2</table></qt>" ,
                                      id, lines.join( QString() ) ),
                              i18n( "Certificate Import Result" ) );
}

void ImportCertificatesCommand::Private::showError( QWidget * parent, const Error & err, const QString & id ) {
    assert( err );
    assert( !err.isCanceled() );
    const QString msg = id.isEmpty()
        ? i18n( "<qt><p>An error occurred while trying "
                "to import the certificate:</p>"
                "<p><b>%1</b></p></qt>",
                QString::fromLocal8Bit( err.asString() ) )
        : i18n( "<qt><p>An error occurred while trying "
                "to import the certificate %1:</p>"
                "<p><b>%2</b></p></qt>",
                id, QString::fromLocal8Bit( err.asString() ) );
    KMessageBox::error( parent, msg, i18n( "Certificate Import Failed" ) );
}

void ImportCertificatesCommand::Private::importResult( const ImportResult & result ) {

    if ( q->sender() == cmsImportJob )
        cmsImportJob = 0;
    if ( q->sender() == pgpImportJob )
        pgpImportJob = 0;

    // ### merge results when gpgme gains copy ctors for result objects

    if ( result.error().code() ) {
        setImportResultProxyModel( result );
        if ( result.error().isCanceled() )
            emit q->canceled();
        else
            showError( result.error() );
    }
    else
        showDetails( result );
    finished();
}

void ImportCertificatesCommand::Private::startImport( GpgME::Protocol protocol, const QByteArray & data, const QString & id ) {
    assert( protocol != UnknownProtocol );
    const Kleo::CryptoBackend::Protocol * const backend = CryptoBackendFactory::instance()->protocol( protocol );
    if ( !backend ) {
        setImportResultProxyModel( ImportResult(), id );
        KMessageBox::error( parentWidgetOrView(), 
                            i18n( "The type of this certificate (%1) is not supported by this Kleopatra installation.",
                                  Formatting::displayName( protocol ) ),
                            i18n( "Certificate Import Failed" ) );
        finished();
        return;
    }
    std::auto_ptr<ImportJob> job( backend->importJob() );
    assert( job.get() );
    connect( job.get(), SIGNAL(result(GpgME::ImportResult)),
             q, SLOT(importResult(GpgME::ImportResult)) );
    connect( job.get(), SIGNAL(progress(QString,int,int)), 
             q, SIGNAL(progress(QString,int,int)) );
    if ( const GpgME::Error err = job->start( data ) ) {
        setImportResultProxyModel( ImportResult( err ), id );
        showError( err, id );
        finished();
    } else if ( err.isCanceled() ) {
        setImportResultProxyModel( ImportResult( err ), id );
        emit q->canceled();
        finished();
    } else {
        if ( protocol == CMS )
            cmsImportJob = job.release();
        else
            pgpImportJob = job.release();
    }
}


void ImportCertificatesCommand::doCancel() {
    if ( d->cmsImportJob )
        d->cmsImportJob->slotCancel();
    if ( d->pgpImportJob )
        d->pgpImportJob->slotCancel();
}

#undef d
#undef q

#include "moc_importcertificatescommand.cpp"
#include "importcertificatescommandc.moc"

