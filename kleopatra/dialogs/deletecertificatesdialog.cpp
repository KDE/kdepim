/* -*- mode: c++; c-basic-offset:4 -*-
    dialogs/deletecertificatesdialog.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2009 Klarälvdalens Datakonsult AB

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

#include "deletecertificatesdialog.h"

#include <view/keytreeview.h>
#include <models/keylistmodel.h>

#include <kleo/stl_util.h>

#include <KLocalizedString>
#include <KMessageBox>
#include <KStandardGuiItem>
#include <QDebug>
#include <KConfigGroup>
#include <KSharedConfig>

#include <QLabel>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QWhatsThis>
#include <QCursor>
#include <QPushButton>
#include <QTreeView>

#include <gpgme++/key.h>

#include <boost/mem_fn.hpp>

#include <cassert>

using namespace Kleo;
using namespace Kleo::Dialogs;
using namespace GpgME;
using namespace boost;

class DeleteCertificatesDialog::Private {
    friend class ::Kleo::Dialogs::DeleteCertificatesDialog;
    DeleteCertificatesDialog * const q;
public:
    explicit Private( DeleteCertificatesDialog * qq )
        : q( qq ),
          ui( q )
    {

    }

    void slotWhatsThisRequested() {
        qDebug();
        if ( QWidget * const widget = qobject_cast<QWidget*>( q->sender() ) )
            if ( !widget->whatsThis().isEmpty() )
                QWhatsThis::showText( QCursor::pos(), widget->whatsThis() );
    }

    void readConfig()
    {
        KConfigGroup dialog( KSharedConfig::openConfig(), "DeleteCertificatesDialog" );
        const QSize size = dialog.readEntry( "Size", QSize(600, 400) );
        if ( size.isValid() ) {
            q->resize( size );
        }
    }

    void writeConfig()
    {
        KConfigGroup dialog( KSharedConfig::openConfig(), "DeleteCertificatesDialog" );
        dialog.writeEntry( "Size", q->size() );
        dialog.sync();
    }

private:
    struct UI {
        QLabel selectedLB;
        KeyTreeView selectedKTV;
        QLabel unselectedLB;
        KeyTreeView unselectedKTV;
        QDialogButtonBox buttonBox;
        QVBoxLayout vlay;

        explicit UI( DeleteCertificatesDialog * qq )
            : selectedLB( i18n( "These are the certificates you have selected for deletion:" ), qq ),
              selectedKTV( qq ),
              unselectedLB( xi18n("These certificates will be deleted even though you did <emphasis>not</emphasis><nl/> "
                                 "explicitly select them (<a href=\"whatsthis://\">Why?</a>):"), qq ),
              unselectedKTV( qq ),
              buttonBox( QDialogButtonBox::Ok|QDialogButtonBox::Cancel ),
              vlay( qq )
        {
            KDAB_SET_OBJECT_NAME( selectedLB );
            KDAB_SET_OBJECT_NAME( selectedKTV );
            KDAB_SET_OBJECT_NAME( unselectedLB );
            KDAB_SET_OBJECT_NAME( unselectedKTV );
            KDAB_SET_OBJECT_NAME( buttonBox );
            KDAB_SET_OBJECT_NAME( vlay );

            vlay.addWidget( &selectedLB );
            vlay.addWidget( &selectedKTV, 1 );
            vlay.addWidget( &unselectedLB );
            vlay.addWidget( &unselectedKTV, 1 );
            vlay.addWidget( &buttonBox );

            const QString unselectedWhatsThis
                = xi18nc( "@info:whatsthis",
                         "<title>Why do you want to delete more certificates than I selected?</title>"
                         "<para>When you delete CA certificates (both root CAs and intermediate CAs), "
                         "the certificates issued by them will also be deleted.</para>"
                         "<para>This can be nicely seen in <application>Kleopatra</application>'s "
                         "hierarchical view mode: In this mode, if you delete a certificate that has "
                         "children, those children will also be deleted. Think of CA certificates as "
                         "folders containing other certificates: When you delete the folder, you "
                         "delete its contents, too.</para>" );

            unselectedLB.setWhatsThis( unselectedWhatsThis );
            unselectedKTV.setWhatsThis( unselectedWhatsThis );

            buttonBox.button( QDialogButtonBox::Ok )->setText( i18nc("@action:button","Delete") );

            connect( &unselectedLB, SIGNAL(linkActivated(QString)), qq, SLOT(slotWhatsThisRequested()) );

            selectedKTV.setFlatModel( AbstractKeyListModel::createFlatKeyListModel( &selectedKTV ) );
            unselectedKTV.setFlatModel( AbstractKeyListModel::createFlatKeyListModel( &unselectedKTV ) );

            selectedKTV.setHierarchicalView( false );
            selectedKTV.view()->setSelectionMode( QAbstractItemView::NoSelection );
            unselectedKTV.setHierarchicalView( false );
            unselectedKTV.view()->setSelectionMode( QAbstractItemView::NoSelection );

            connect( &buttonBox, SIGNAL(accepted()), qq, SLOT(accept()) );
            connect( &buttonBox, SIGNAL(rejected()), qq, SLOT(reject()) );
        }
    } ui;
};

DeleteCertificatesDialog::DeleteCertificatesDialog( QWidget * p, Qt::WindowFlags f )
    : QDialog( p, f ), d( new Private( this ) )
{
    d->readConfig();
}

DeleteCertificatesDialog::~DeleteCertificatesDialog()
{
    d->writeConfig();
}


void DeleteCertificatesDialog::setSelectedKeys( const std::vector<Key> & keys ) {
    d->ui.selectedKTV.setKeys( keys );
}

void DeleteCertificatesDialog::setUnselectedKeys( const std::vector<Key> & keys ) {
    d->ui.unselectedLB .setVisible( !keys.empty() );
    d->ui.unselectedKTV.setVisible( !keys.empty() );
    d->ui.unselectedKTV.setKeys( keys );
}

std::vector<Key> DeleteCertificatesDialog::keys() const {
    const std::vector<Key> sel = d->ui.selectedKTV.keys();
    const std::vector<Key> uns = d->ui.unselectedKTV.keys();
    std::vector<Key> result;
    result.reserve( sel.size() + uns.size() );
    result.insert( result.end(), sel.begin(), sel.end() );
    result.insert( result.end(), uns.begin(), uns.end() );
    return result;
}

void DeleteCertificatesDialog::accept() {

    const std::vector<Key> sel = d->ui.selectedKTV.keys();
    const std::vector<Key> uns = d->ui.unselectedKTV.keys();

    const uint secret = kdtools::count_if( sel, mem_fn( &Key::hasSecret ) )
                      + kdtools::count_if( uns, mem_fn( &Key::hasSecret ) );
    const uint total  = sel.size() + uns.size();

    int ret = KMessageBox::Continue;
    if ( secret )
        ret = KMessageBox::warningContinueCancel( this,
                secret == total
                ? i18np("The certificate to be deleted is your own. "
                        "It contains private key material, "
                        "which is needed to decrypt past communication "
                        "encrypted to the certificate, and should therefore "
                        "not be deleted.",

                        "All of the certificates to be deleted "
                        "are your own. "
                        "They contain private key material, "
                        "which is needed to decrypt past communication "
                        "encrypted to the certificate, and should therefore "
                        "not be deleted.",

                        secret )
                : i18np("One of the certificates to be deleted "
                        "is your own. "
                        "It contains private key material, "
                        "which is needed to decrypt past communication "
                        "encrypted to the certificate, and should therefore "
                        "not be deleted.",

                        "Some of the certificates to be deleted "
                        "are your own. "
                        "They contain private key material, "
                        "which is needed to decrypt past communication "
                        "encrypted to the certificate, and should therefore "
                        "not be deleted.",

                        secret ),
                i18n("Secret Key Deletion"),
                KStandardGuiItem::guiItem( KStandardGuiItem::Delete ),
                KStandardGuiItem::cancel(), QString(), KMessageBox::Notify|KMessageBox::Dangerous );

    if ( ret == KMessageBox::Continue )
        QDialog::accept();
    else
        QDialog::reject();
}

#include "moc_deletecertificatesdialog.cpp"
