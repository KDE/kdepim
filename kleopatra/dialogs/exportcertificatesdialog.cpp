/* -*- mode: c++; c-basic-offset:4 -*-
    dialogs/exportcertificatesdialog.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2007 Klarälvdalens Datakonsult AB

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

#include "exportcertificatesdialog.h"

#include "libkleo/ui/filenamerequester.h"

#include <KGuiItem>
#include <KLocalizedString>
#include <KPushButton>

#include <QGridLayout>
#include <QLabel>

using namespace Kleo;
using namespace Kleo::Dialogs;

class ExportCertificatesDialog::Private {
    friend class ::Kleo::Dialogs::ExportCertificatesDialog;
    ExportCertificatesDialog * const q;
public:
    explicit Private( ExportCertificatesDialog * qq );
    ~Private();
    void fileNamesChanged();

private:
    FileNameRequester* pgpRequester;
    FileNameRequester* cmsRequester;
};


ExportCertificatesDialog::Private::Private( ExportCertificatesDialog * qq )
  : q( qq )
{
    q->setButtons( KDialog::Ok | KDialog::Cancel );
    q->setButtonGuiItem( KDialog::Ok, KGuiItem( i18n( "Export" ) ) );
    QWidget* const main = new QWidget;
    QGridLayout* const grid = new QGridLayout( main );
    QLabel* const pgpLabel = new QLabel;
    pgpLabel->setText( i18n(" OpenPGP export file:" ) );
    grid->addWidget( pgpLabel, 0, 0 );     
    pgpRequester = new FileNameRequester;
    connect( pgpRequester, SIGNAL(fileNameChanged(QString)), q, SLOT(fileNamesChanged()) );
    grid->addWidget( pgpRequester, 0, 1 );
    QLabel* const cmsLabel = new QLabel;
    cmsLabel->setText( i18n( "S/MIME export file:" ) );
    grid->addWidget( cmsLabel, 1, 0 );
    cmsRequester = new FileNameRequester;
    connect( cmsRequester, SIGNAL(fileNameChanged(QString)), q, SLOT(fileNamesChanged()) );
    grid->addWidget( cmsRequester, 1, 1 );
    q->setMainWidget( main );
    fileNamesChanged();
}

ExportCertificatesDialog::Private::~Private() {}



ExportCertificatesDialog::ExportCertificatesDialog( QWidget * parent, Qt::WindowFlags f )
  : KDialog( parent, f ), d( new Private( this ) )
{
    
}

void ExportCertificatesDialog::Private::fileNamesChanged()
{
    q->button( KDialog::Ok )->setEnabled( !pgpRequester->fileName().isEmpty() && !cmsRequester->fileName().isEmpty() );
}

ExportCertificatesDialog::~ExportCertificatesDialog() {}


void ExportCertificatesDialog::setOpenPgpExportFileName( const QString & fileName )
{
    d->pgpRequester->setFileName( fileName );
}

QString ExportCertificatesDialog::openPgpExportFileName() const
{
    return d->pgpRequester->fileName();
}

void ExportCertificatesDialog::setCmsExportFileName( const QString & fileName )
{
    d->cmsRequester->setFileName( fileName );
}

QString ExportCertificatesDialog::cmsExportFileName() const
{
    return d->cmsRequester->fileName();
}

#include "moc_exportcertificatesdialog.cpp"

