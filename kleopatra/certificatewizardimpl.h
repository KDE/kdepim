/*
    certificatewizardimpl.h

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2001,2002,2004 Klar�vdalens Datakonsult AB

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

#ifndef CERTIFICATEWIZARDIMPL_H
#define CERTIFICATEWIZARDIMPL_H
#include "ui_certificatewizard.h"

#include <q3valuevector.h>
#include <Q3Wizard>
#include <QLineEdit>
#include <kurl.h>

namespace GpgME {
  class KeyGenerationResult;
}
namespace KIO {
  class Job;
}

class KJob;

class CertificateWizard : public Q3Wizard, public Ui::CertificateWizard
{
public:
  CertificateWizard( QWidget *parent ) : Q3Wizard( parent ) {
    setupUi( this );
  }
};


class CertificateWizardImpl : public CertificateWizard
{
    Q_OBJECT

public:
    CertificateWizardImpl( QWidget* parent = 0);
    ~CertificateWizardImpl();

    bool sendToCA() const;
    QString caEMailAddress() const;
    KUrl saveFileUrl() const;

    typedef QPair<QString, QLineEdit*> StringLEPair;
    typedef QVector< StringLEPair > AttrPairList;

public:
    virtual void showPage( QWidget * page );
    virtual void accept();

private slots:
    void slotGenerateCertificate();
    void slotResult( const GpgME::KeyGenerationResult & res, const QByteArray & keyData );
    void slotSetValuesFromWhoAmI();
    void slotEnablePersonalDataPageExit();
    void slotURLSelected( const KUrl& );

    void slotHelpClicked();

    void slotUploadResult( KJob* );

private:
    void createPersonalDataPage();
    void sendCertificate( const QString& email, const QByteArray& certificateData );

private:
    AttrPairList _attrPairList;
    QByteArray _keyData;
};

#endif // CERTIFICATEWIZARDIMPL_H
