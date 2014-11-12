/*
    messagebox.cpp

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

#include "messagebox.h"
#include "messagebox_p.h"

#include "kleo/job.h"

#include <gpgme++/signingresult.h>
#include <gpgme++/encryptionresult.h>

#include "pimcommon/texteditor/richtexteditor/richtexteditor.h"


#ifndef KDEPIM_ONLY_KLEO
# include <kfiledialog.h>
#else
# include <QFileDialog>
#endif

#include <kdialog.h>
#include <klocale.h>
#include <ksavefile.h>
#include <kguiitem.h>
#include <kdebug.h>
#include <ktextedit.h>

#include <qtextstream.h>

#include <gpg-error.h>

using namespace Kleo;
using namespace Kleo::Private;
using namespace GpgME;

namespace {

#ifndef QT_NO_FILEDIALOG
static KGuiItem KGuiItem_save() {
    return KGuiItem( i18n("&Save to Disk..."), QLatin1String("document-save-as") );
}
#endif

#ifndef QT_NO_CLIPBOARD
static KGuiItem KGuiItem_copy() {
    return KGuiItem( i18n("&Copy to Clipboard"), QLatin1String("edit-copy"), i18n("Copy Audit Log to Clipboard") );
}
#endif

static KGuiItem KGuiItem_showAuditLog() {
    return KGuiItem( i18n("&Show Audit Log") ); // "view_log"?
}

} // anon namespace

AuditLogViewer::AuditLogViewer( const QString & log, QWidget * parent, Qt::WindowFlags f )
    : KDialog( parent, f ),
      m_log( /* sic */ ),
      m_textEdit( new PimCommon::RichTextEditorWidget( this ) )
{
    setCaption( i18n("View GnuPG Audit Log") );
    setButtons( Close
#ifndef QT_NO_FILEDIALOG
                |User1
#endif
#ifndef QT_NO_CLIPBOARD
                |User2
#endif
                );
    setDefaultButton( Close );
#ifndef QT_NO_FILEDIALOG
    setButtonGuiItem( User1, KGuiItem_save() );
#endif
#ifndef QT_NO_CLIPBOARD
    setButtonGuiItem( User2, KGuiItem_copy() );
#endif
    showButtonSeparator( false );
    setModal( false );
    setMainWidget( m_textEdit );
    m_textEdit->setObjectName( QLatin1String("m_textEdit") );
    m_textEdit->setReadOnly( true );
    setAuditLog( log );

#ifndef QT_NO_FILEDIALOG
    connect( this, SIGNAL(user1Clicked()), SLOT(slotUser1()) );
#endif
#ifndef QT_NO_CLIPBOARD
    connect( this, SIGNAL(user2Clicked()), SLOT(slotUser2()) );
#endif
    readConfig();
}

AuditLogViewer::~AuditLogViewer()
{
    writeConfig();
}

void AuditLogViewer::setAuditLog( const QString & log ) {
  if ( log == m_log )
    return;
  m_log = log;
  m_textEdit->setHtml( QLatin1String("<qt>") + log + QLatin1String("</qt>") );
}

#ifndef QT_NO_FILEDIALOG
void AuditLogViewer::slotUser1() {
#ifndef KDEPIM_ONLY_KLEO
    const QString fileName = KFileDialog::getSaveFileName( QString(), QString(),
                                                           this, i18n("Choose File to Save GnuPG Audit Log to") );
#else
    const QString fileName = QFileDialog::getSaveFileName( this, i18n("Choose File to Save GnuPG Audit Log to") );
#endif
    if ( fileName.isEmpty() )
        return;

    KSaveFile file( fileName );

    if ( file.open() ) {
        QTextStream s( &file );
        s << "<html><head>";
        if ( !windowTitle().isEmpty() ) {
          s << "\n<title>"
            << Qt::escape( windowTitle() )
            << "</title>\n";
        }
        s << "</head><body>\n"
          << m_log
          << "\n</body></html>" << endl;
        s.flush();
        file.finalize();
    }

    if ( const int err = file.error() )
        KMessageBox::error( this, i18n("Could not save to file \"%1\": %2",
                            file.fileName(), QString::fromLocal8Bit( strerror( err ) ) ),
                            i18n("File Save Error") );
}
#endif // QT_NO_FILEDIALOG

#ifndef QT_NO_CLIPBOARD
void AuditLogViewer::slotUser2() {
    m_textEdit->editor()->selectAll();
    m_textEdit->editor()->copy();
    m_textEdit->editor()->textCursor().clearSelection();
}
#endif // QT_NO_CLIPBOARD

void AuditLogViewer::readConfig()
{
    KConfigGroup group( KGlobal::config(), "AuditLogViewer" );
    const QSize size = group.readEntry( "Size", QSize() );
    if ( size.isValid() ) {
        resize( size );
    } else {
        resize( 600, 400 );
    }
}

void AuditLogViewer::writeConfig()
{
    KConfigGroup group( KGlobal::config(), "AuditLogViewer" );
    group.writeEntry( "Size", size() );
    group.sync();
}


// static
void MessageBox::auditLog( QWidget * parent, const Job * job, const QString & caption ) {

    if ( !job )
        return;

    if ( !GpgME::hasFeature( AuditLogFeature ) || !job->isAuditLogSupported() ) {
        KMessageBox::information( parent, i18n("Your system does not have support for GnuPG Audit Logs"),
                                  i18n("System Error") );
        return;
    }

    const GpgME::Error err = job->auditLogError();

    if ( err && err.code() != GPG_ERR_NO_DATA ) {
        KMessageBox::information( parent, i18n("An error occurred while trying to retrieve the GnuPG Audit Log:\n%1",
                                  QString::fromLocal8Bit( err.asString() ) ),
                                  i18n("GnuPG Audit Log Error") );
        return;
    }

    const QString log = job->auditLogAsHtml();

    if ( log.isEmpty() ) {
        KMessageBox::information( parent, i18n("No GnuPG Audit Log available for this operation."),
                                  i18n("No GnuPG Audit Log") );
        return;
    }

    auditLog( parent, log, caption );
}

// static
void MessageBox::auditLog( QWidget * parent, const QString & log, const QString & caption ) {
    AuditLogViewer * const alv = new AuditLogViewer( log, parent );
    alv->setAttribute( Qt::WA_DeleteOnClose );
    alv->setObjectName( QLatin1String("alv") );
    alv->setCaption( caption );
    alv->show();
}

// static
void MessageBox::auditLog( QWidget * parent, const Job * job ) {
    auditLog( parent, job, i18n("GnuPG Audit Log Viewer") );
}

// static
void MessageBox::auditLog( QWidget * parent, const QString & log ) {
    auditLog( parent, log, i18n("GnuPG Audit Log Viewer") );
}

static QString to_information_string( const SigningResult & result ) {
    return result.error()
        ? i18n("Signing failed: %1", QString::fromLocal8Bit( result.error().asString() ) )
        : i18n("Signing successful") ;
}

static QString to_error_string( const SigningResult & result ) {
    return to_information_string( result );
}

static QString to_information_string( const EncryptionResult & result ) {
    return result.error()
        ? i18n("Encryption failed: %1", QString::fromLocal8Bit( result.error().asString() ) )
        : i18n("Encryption successful") ;
}

static QString to_error_string( const EncryptionResult & result ) {
    return to_information_string( result );
}

static QString to_information_string( const SigningResult & sresult, const EncryptionResult & eresult ) {
    return to_information_string( sresult ) + QLatin1Char('\n') + to_information_string( eresult );
}

static QString to_error_string( const SigningResult & sresult, const EncryptionResult & eresult ) {
    return to_information_string( sresult, eresult );
}

// static
void MessageBox::information( QWidget * parent, const SigningResult & result, const Job * job, KMessageBox::Options options ) {
    information( parent, result, job, i18n("Signing Result"), options );
}

// static
void MessageBox::information( QWidget * parent, const SigningResult & result, const Job * job, const QString & caption, KMessageBox::Options options ) {
    make( parent, QMessageBox::Information, to_information_string( result ), job, caption, options );
}

// static
void MessageBox::error( QWidget * parent, const SigningResult & result, const Job * job, KMessageBox::Options options ) {
    error( parent, result, job, i18n("Signing Error"), options );
}

// static
void MessageBox::error( QWidget * parent, const SigningResult & result, const Job * job, const QString & caption, KMessageBox::Options options ) {
    make( parent, QMessageBox::Critical, to_error_string( result ), job, caption, options );
}

// static
void MessageBox::information( QWidget * parent, const EncryptionResult & result, const Job * job, KMessageBox::Options options ) {
    information( parent, result, job, i18n("Encryption Result"), options );
}

// static
void MessageBox::information( QWidget * parent, const EncryptionResult & result, const Job * job, const QString & caption, KMessageBox::Options options ) {
    make( parent, QMessageBox::Information, to_information_string( result ), job, caption, options );
}

// static
void MessageBox::error( QWidget * parent, const EncryptionResult & result, const Job * job, KMessageBox::Options options ) {
    error( parent, result, job, i18n("Encryption Error"), options );
}

// static
void MessageBox::error( QWidget * parent, const EncryptionResult & result, const Job * job, const QString & caption, KMessageBox::Options options ) {
    make( parent, QMessageBox::Critical, to_error_string( result ), job, caption, options );
}

// static
void MessageBox::information( QWidget * parent, const SigningResult & sresult, const EncryptionResult & eresult, const Job * job, KMessageBox::Options options ) {
    information( parent, sresult, eresult, job, i18n("Encryption Result"), options );
}

// static
void MessageBox::information( QWidget * parent, const SigningResult & sresult, const EncryptionResult & eresult, const Job * job, const QString & caption, KMessageBox::Options options ) {
    make( parent, QMessageBox::Information, to_information_string( sresult, eresult ), job, caption, options );
}

// static
void MessageBox::error( QWidget * parent, const SigningResult & sresult, const EncryptionResult & eresult, const Job * job, KMessageBox::Options options ) {
    error( parent, sresult, eresult, job, i18n("Encryption Error"), options );
}

// static
void MessageBox::error( QWidget * parent, const SigningResult & sresult, const EncryptionResult & eresult, const Job * job, const QString & caption, KMessageBox::Options options ) {
    make( parent, QMessageBox::Critical, to_error_string( sresult, eresult ), job, caption, options );
}

// static
bool MessageBox::showAuditLogButton( const Kleo::Job * job ) {
    if ( !job ) {
        kDebug(5150) << "not showing audit log button (no job instance)";
        return false;
    }
    if ( !GpgME::hasFeature( GpgME::AuditLogFeature ) ) {
        kDebug(5150) << "not showing audit log button (gpgme too old)";
        return false;
    }
    if ( !job->isAuditLogSupported() ) {
        kDebug(5150) << "not showing audit log button (not supported)";
        return false;
    }
    if ( job->auditLogError().code() == GPG_ERR_NO_DATA ) {
        kDebug(5150) << "not showing audit log button (GPG_ERR_NO_DATA)";
        return false;
    }
    if ( !job->auditLogError() && job->auditLogAsHtml().isEmpty() ) {
        kDebug(5150) << "not showing audit log button (success, but result empty)";
        return false;
    }
    return true;
}


// static
void MessageBox::make( QWidget * parent, QMessageBox::Icon icon, const QString & text, const Job * job, const QString & caption, KMessageBox::Options options ) {
    KDialog * dialog = new KDialog( parent );
    dialog->setCaption( caption );
    dialog->setButtons( showAuditLogButton( job ) ? ( KDialog::Yes | KDialog::No ) : KDialog::Yes );
    dialog->setDefaultButton( KDialog::Yes );
    dialog->setEscapeButton( KDialog::Yes );
    dialog->setObjectName( QLatin1String("error") );
    dialog->setModal( true );
    dialog->showButtonSeparator( true );
    dialog->setButtonGuiItem( KDialog::Yes, KStandardGuiItem::ok() );
    if ( GpgME::hasFeature( GpgME::AuditLogFeature ) )
      dialog->setButtonGuiItem( KDialog::No, KGuiItem_showAuditLog() );

    if ( options & KMessageBox::PlainCaption )
        dialog->setPlainCaption( caption );

    if ( KDialog::No == KMessageBox::createKMessageBox( dialog, icon, text, QStringList(), QString::null, 0, options ) )
        auditLog( 0, job );
}

#include "moc_messagebox_p.cpp"
