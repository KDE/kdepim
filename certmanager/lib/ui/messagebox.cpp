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

    ( new AuditLogViewer( "<qt>" + log + "</qt>", parent, "alv", Qt::WDestructiveClose ) )->show();
}
