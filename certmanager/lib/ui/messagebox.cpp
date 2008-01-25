/*
    messagebox.cpp

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2004 Klar�lvdalens Datakonsult AB

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "messagebox.h"

#include "kleo/job.h"

#include <gpgmepp/signingresult.h>
#include <gpgmepp/encryptionresult.h>

#include <kfiledialog.h>
#include <kdialogbase.h>
#include <klocale.h>
#include <ksavefile.h>
#include <kguiitem.h>

#include <qtextedit.h>
#include <qtextstream.h>
#include <qvbox.h>

using namespace Kleo;
using namespace GpgME;

namespace {

static KGuiItem KGuiItem_save() {
    return KGuiItem( i18n("&Save to Disk..."), "filesaveas" );
}

static KGuiItem KGuiItem_copy() {
    return KGuiItem( i18n("&Copy to Clipboard"), "editcopy", i18n("Copy Audit Log to Clipboard") );
}

static KGuiItem KGuiItem_showAuditLog() {
    return KGuiItem( i18n("&Show Audit Log") ); // "view_log"?
}

class AuditLogViewer : public KDialogBase {
    // Q_OBJECT
public:
    explicit AuditLogViewer( const QString & log, QWidget * parent=0, const char * name=0, WFlags f=0 )
        : KDialogBase( parent, name, false, i18n("View GnuPG Audit Log"),
                       Close|User1|User2, Close, false, KGuiItem_save(), KGuiItem_copy() ),
          m_textEdit( new QTextEdit( this, "m_textEdit" ) )
    {
        setWFlags( f );
        setMainWidget( m_textEdit );
        m_textEdit->setTextFormat( QTextEdit::RichText );
        m_textEdit->setReadOnly( true );
        setAuditLog( log );
    }
    ~AuditLogViewer() {}

    void setAuditLog( const QString & log ) {
        m_textEdit->setText( log );
    }

private:
    void slotUser1() {
        const QString fileName = KFileDialog::getSaveFileName( QString(), QString(),
                                                               this, i18n("Choose File to Save GnuPG Audit Log to") );
        if ( fileName.isEmpty() )
            return;

        KSaveFile file( fileName );

        if ( QTextStream * const s = file.textStream() ) {
            *s << m_textEdit->text() << endl;
            file.close();
        }

        if ( const int err = file.status() )
            KMessageBox::error( this, i18n("Couldn't save to file \"%1\": %2")
                                .arg( file.name(), QString::fromLocal8Bit( strerror( err ) ) ),
                                i18n("File Save Error") );
    }
    void slotUser2() {
        m_textEdit->selectAll();
        m_textEdit->copy();
        m_textEdit->selectAll( false );
    }

private:
    QTextEdit * m_textEdit;
};

} // anon namespace

// static
void MessageBox::auditLog( QWidget * parent, const Job * job, const QString & caption ) {

    if ( !job )
        return;

    if ( !GpgME::hasFeature( AuditLogFeature ) ) {
        KMessageBox::information( parent, i18n("Your system does not have support for GnuPG Audit Logs"),
                                  i18n("System Error") );
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
    AuditLogViewer * const alv = new AuditLogViewer( "<qt>" + log + "</qt>", parent, "alv", Qt::WDestructiveClose );
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
        ? i18n("Signing failed: %1").arg( QString::fromLocal8Bit( result.error().asString() ) )
        : i18n("Signing successful") ;
}

static QString to_error_string( const SigningResult & result ) {
    return to_information_string( result );
}

static QString to_information_string( const EncryptionResult & result ) {
    return result.error()
        ? i18n("Encryption failed: %1").arg( QString::fromLocal8Bit( result.error().asString() ) )
        : i18n("Encryption successful") ;
}

static QString to_error_string( const EncryptionResult & result ) {
    return to_information_string( result );
}

static QString to_information_string( const SigningResult & sresult, const EncryptionResult & eresult ) {
    return to_information_string( sresult ) + '\n' + to_information_string( eresult );
}

static QString to_error_string( const SigningResult & sresult, const EncryptionResult & eresult ) {
    return to_information_string( sresult, eresult );
}

// static
void MessageBox::information( QWidget * parent, const SigningResult & result, const Job * job, int options ) {
    information( parent, result, job, i18n("Signing Result"), options );
}

// static
void MessageBox::information( QWidget * parent, const SigningResult & result, const Job * job, const QString & caption, int options ) {
    make( parent, QMessageBox::Information, to_information_string( result ), job, caption, options );
}

// static
void MessageBox::error( QWidget * parent, const SigningResult & result, const Job * job, int options ) {
    error( parent, result, job, i18n("Signing Error"), options );
}

// static
void MessageBox::error( QWidget * parent, const SigningResult & result, const Job * job, const QString & caption, int options ) {
    make( parent, QMessageBox::Critical, to_error_string( result ), job, caption, options );
}

// static
void MessageBox::information( QWidget * parent, const EncryptionResult & result, const Job * job, int options ) {
    information( parent, result, job, i18n("Encryption Result"), options );
}

// static
void MessageBox::information( QWidget * parent, const EncryptionResult & result, const Job * job, const QString & caption, int options ) {
    make( parent, QMessageBox::Information, to_information_string( result ), job, caption, options );
}

// static
void MessageBox::error( QWidget * parent, const EncryptionResult & result, const Job * job, int options ) {
    error( parent, result, job, i18n("Encryption Error"), options );
}

// static
void MessageBox::error( QWidget * parent, const EncryptionResult & result, const Job * job, const QString & caption, int options ) {
    make( parent, QMessageBox::Critical, to_error_string( result ), job, caption, options );
}

// static
void MessageBox::information( QWidget * parent, const SigningResult & sresult, const EncryptionResult & eresult, const Job * job, int options ) {
    information( parent, sresult, eresult, job, i18n("Encryption Result"), options );
}

// static
void MessageBox::information( QWidget * parent, const SigningResult & sresult, const EncryptionResult & eresult, const Job * job, const QString & caption, int options ) {
    make( parent, QMessageBox::Information, to_information_string( sresult, eresult ), job, caption, options );
}

// static
void MessageBox::error( QWidget * parent, const SigningResult & sresult, const EncryptionResult & eresult, const Job * job, int options ) {
    error( parent, sresult, eresult, job, i18n("Encryption Error"), options );
}

// static
void MessageBox::error( QWidget * parent, const SigningResult & sresult, const EncryptionResult & eresult, const Job * job, const QString & caption, int options ) {
    make( parent, QMessageBox::Critical, to_error_string( sresult, eresult ), job, caption, options );
}

// static
void MessageBox::make( QWidget * parent, QMessageBox::Icon icon, const QString & text, const Job * job, const QString & caption, int options ) {
    KDialogBase * dialog = GpgME::hasFeature( GpgME::AuditLogFeature )
        ? new KDialogBase( caption, KDialogBase::Yes | KDialogBase::No,
                           KDialogBase::Yes, KDialogBase::Yes,
                           parent, "error", true, true,
                           KStdGuiItem::ok(), KGuiItem_showAuditLog() )
        : new KDialogBase( caption, KDialogBase::Yes,
                           KDialogBase::Yes, KDialogBase::Yes,
                           parent, "error", true, true,
                           KStdGuiItem::ok() ) ;
    if ( options & KMessageBox::PlainCaption )
        dialog->setPlainCaption( caption );

    if ( KDialogBase::No == KMessageBox::createKMessageBox( dialog, icon, text, QStringList(), QString::null, 0, options ) )
        auditLog( 0, job );
}
