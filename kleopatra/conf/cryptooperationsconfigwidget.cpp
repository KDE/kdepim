/*
    cryptooperationsconfigwidget.cpp

    This file is part of kleopatra, the KDE key manager
    Copyright (c) 2010 Klarälvdalens Datakonsult AB

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

#include <config-kleopatra.h>

#include "cryptooperationsconfigwidget.h"
#include "ui_cryptooperationsconfigwidget.h"

#include "emailoperationspreferences.h"
#include "fileoperationspreferences.h"

#include <kleo/checksumdefinition.h>

#include <kconfig.h>
#include <kglobal.h>
#include <klocale.h>
#include <kconfiggroup.h>

#include <QLayout>

#include <boost/shared_ptr.hpp>

using namespace Kleo;
using namespace Kleo::Config;
using namespace boost;

class CryptoOperationsConfigWidget::Private {
    friend class ::Kleo::Config::CryptoOperationsConfigWidget;
    CryptoOperationsConfigWidget * const q;
public:
    explicit Private( CryptoOperationsConfigWidget * qq )
        : q( qq ), ui( q ) {}

private:
    struct UI : Ui_CryptoOperationsConfigWidget {

        explicit UI( CryptoOperationsConfigWidget * q )
            : Ui_CryptoOperationsConfigWidget()
        {
            setupUi( q );

            if ( QLayout * const l = q->layout() )
                l->setMargin( 0 );

            connect( quickSignCB,    SIGNAL(toggled(bool)), q, SIGNAL(changed()) );
            connect( quickEncryptCB, SIGNAL(toggled(bool)), q, SIGNAL(changed()) );
            connect( checksumDefinitionCB, SIGNAL(currentIndexChanged(int)), q, SIGNAL(changed()) );
            connect( pgpFileExtCB, SIGNAL(toggled(bool)), q, SIGNAL(changed()) );
        }

    } ui;

};

CryptoOperationsConfigWidget::CryptoOperationsConfigWidget( QWidget * p, Qt::WindowFlags f )
    : QWidget( p, f ), d( new Private( this ) )
{
//    load();
}


CryptoOperationsConfigWidget::~CryptoOperationsConfigWidget() {}

void CryptoOperationsConfigWidget::defaults() {
    EMailOperationsPreferences emailPrefs;
    emailPrefs.setDefaults();
    d->ui.quickSignCB->setChecked( emailPrefs.quickSignEMail() );
    d->ui.quickEncryptCB->setChecked( emailPrefs.quickEncryptEMail() );

    FileOperationsPreferences filePrefs;
    filePrefs.setDefaults();
    d->ui.pgpFileExtCB->setChecked( filePrefs.usePGPFileExt() );

    if ( d->ui.checksumDefinitionCB->count() )
        d->ui.checksumDefinitionCB->setCurrentIndex( 0 );
}

Q_DECLARE_METATYPE( boost::shared_ptr<Kleo::ChecksumDefinition> )

void CryptoOperationsConfigWidget::load() {

    const EMailOperationsPreferences emailPrefs;
    d->ui.quickSignCB   ->setChecked( emailPrefs.quickSignEMail()    );
    d->ui.quickEncryptCB->setChecked( emailPrefs.quickEncryptEMail() );

    const FileOperationsPreferences filePrefs;
    d->ui.pgpFileExtCB->setChecked( filePrefs.usePGPFileExt() );

    const std::vector< shared_ptr<ChecksumDefinition> > cds = ChecksumDefinition::getChecksumDefinitions();
    const shared_ptr<ChecksumDefinition> default_cd = ChecksumDefinition::getDefaultChecksumDefinition( cds );

    d->ui.checksumDefinitionCB->clear();

    Q_FOREACH( const shared_ptr<ChecksumDefinition> & cd, cds ) {
        d->ui.checksumDefinitionCB->addItem( cd->label(), qVariantFromValue( cd  ) );
        if ( cd == default_cd )
            d->ui.checksumDefinitionCB->setCurrentIndex( d->ui.checksumDefinitionCB->count() - 1 );
    }
}

void CryptoOperationsConfigWidget::save() {

    EMailOperationsPreferences emailPrefs;
    emailPrefs.setQuickSignEMail   ( d->ui.quickSignCB   ->isChecked() );
    emailPrefs.setQuickEncryptEMail( d->ui.quickEncryptCB->isChecked() );
    emailPrefs.writeConfig();

    FileOperationsPreferences filePrefs;
    filePrefs.setUsePGPFileExt( d->ui.pgpFileExtCB->isChecked() );
    filePrefs.writeConfig();

    const int idx = d->ui.checksumDefinitionCB->currentIndex();
    if ( idx < 0 )
        return; // ### pick first?
    const shared_ptr<ChecksumDefinition> cd = qvariant_cast< shared_ptr<ChecksumDefinition> >( d->ui.checksumDefinitionCB->itemData( idx ) );
    ChecksumDefinition::setDefaultChecksumDefinition( cd );

}

