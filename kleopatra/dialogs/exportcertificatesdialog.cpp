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

#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QVBoxLayout>

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
    QPushButton *mOkButton;
};


ExportCertificatesDialog::Private::Private( ExportCertificatesDialog * qq )
  : q( qq )
{
    QVBoxLayout *mainLayout = new QVBoxLayout;
    q->setLayout(mainLayout);
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    mOkButton = buttonBox->button(QDialogButtonBox::Ok);
    mOkButton->setDefault(true);
    mOkButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, SIGNAL(accepted()), q, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), q, SLOT(reject()));

    KGuiItem::assign(mOkButton, KGuiItem( i18n( "Export" ) ) );
    QWidget* const main = new QWidget;
    mainLayout->addWidget(main);
    mainLayout->addWidget(buttonBox);

    QFormLayout *layout = new QFormLayout;
    main->setLayout(layout);

    QLabel* const pgpLabel = new QLabel;
    pgpLabel->setText( i18n(" OpenPGP export file:" ) );
    pgpRequester = new FileNameRequester;
    connect( pgpRequester, SIGNAL(fileNameChanged(QString)), q, SLOT(fileNamesChanged()) );
    layout->addRow(pgpLabel, pgpRequester);

    QLabel* const cmsLabel = new QLabel;
    cmsLabel->setText( i18n( "S/MIME export file:" ) );
    cmsRequester = new FileNameRequester;
    layout->addRow(cmsLabel, cmsRequester);

    connect( cmsRequester, SIGNAL(fileNameChanged(QString)), q, SLOT(fileNamesChanged()) );
    fileNamesChanged();
}

ExportCertificatesDialog::Private::~Private() {}



ExportCertificatesDialog::ExportCertificatesDialog( QWidget * parent, Qt::WindowFlags f )
  : QDialog( parent, f ), d( new Private( this ) )
{
    
}

void ExportCertificatesDialog::Private::fileNamesChanged()
{
    mOkButton->setEnabled( !pgpRequester->fileName().isEmpty() && !cmsRequester->fileName().isEmpty() );
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

