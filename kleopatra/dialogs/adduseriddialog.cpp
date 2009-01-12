/* -*- mode: c++; c-basic-offset:4 -*-
    dialogs/adduseriddialog.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2008 Klarälvdalens Datakonsult AB

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

#include "adduseriddialog.h"

#include "ui_adduseriddialog.h"

#include <utils/validation.h>

#include <QString>
#include <QStringList>
#include <QPushButton>
#include <QValidator>

#include <cassert>

using namespace Kleo;
using namespace Kleo::Dialogs;

class AddUserIDDialog::Private {
    friend class ::Kleo::Dialogs::AddUserIDDialog;
    AddUserIDDialog * const q;
public:
    explicit Private( AddUserIDDialog * qq )
        : q( qq ),
          ui( q )
    {

    }

private:
    void slotUserIDChanged();

private:
    struct UI : public Ui_AddUserIDDialog {
        explicit UI( AddUserIDDialog * qq )
            : Ui_AddUserIDDialog()
        {
            setupUi( qq );

            nameLE->setValidator( Validation::pgpName( nameLE ) );
            emailLE->setValidator( Validation::email( emailLE ) );
            commentLE->setValidator( Validation::pgpComment( commentLE ) );
        }

        QPushButton * okPB() const {
            return buttonBox->button( QDialogButtonBox::Ok );
        }
    } ui;
};

AddUserIDDialog::AddUserIDDialog( QWidget * p, Qt::WindowFlags f )
    : QDialog( p, f ), d( new Private( this ) )
{

}

AddUserIDDialog::~AddUserIDDialog() {}


void AddUserIDDialog::setName( const QString & name ) {
    d->ui.nameLE->setText( name );
}

QString AddUserIDDialog::name() const {
    return d->ui.nameLE->text().trimmed();
}

void AddUserIDDialog::setEmail( const QString & email ) {
    d->ui.emailLE->setText( email );
}

QString AddUserIDDialog::email() const {
    return d->ui.emailLE->text().trimmed();
}

void AddUserIDDialog::setComment( const QString & comment ) {
    d->ui.commentLE->setText( comment );
}

QString AddUserIDDialog::comment() const {
    return d->ui.commentLE->text().trimmed();
}

static bool has_intermediate_input( const QLineEdit * le ) {
    QString text = le->text();
    int pos = le->cursorPosition();
    const QValidator * const v = le->validator();
    return !v || v->validate( text, pos ) == QValidator::Intermediate ;
}

void AddUserIDDialog::Private::slotUserIDChanged() {

    bool ok = false;
    QString error;

    if ( ui.nameLE->text().trimmed().isEmpty() )
        error = i18nc("@info", "<interface>Real name</interface> is required, but missing.");
    else if ( !ui.nameLE->hasAcceptableInput() )
        error = i18nc("@info", "<interface>Real name</interface> must be at least 5 characters long.");
    else if ( ui.emailLE->text().trimmed().isEmpty() )
        error = i18nc("@info", "<interface>EMail address</interface> is required, but missing.");
    else if ( has_intermediate_input( ui.emailLE ) )
        error = i18nc("@info", "<interface>EMail address</interface> is incomplete." );
    else if ( !ui.emailLE->hasAcceptableInput() )
        error = i18nc("@info", "<interface>EMail address</interface> is invalid.");
    else if ( !ui.commentLE->hasAcceptableInput() )
        error = i18nc("@info", "<interface>Comment</interface> contains invalid characters.");
    else
        ok = true;

    ui.okPB()->setEnabled( ok );
    ui.errorLB->setText( error );

    const QString name = q->name();
    const QString email = q->email();
    const QString comment = q->comment();

    QStringList parts;
    if ( !name.isEmpty() )
        parts.push_back( name );
    if ( !comment.isEmpty() )
        parts.push_back( QLatin1Char( '(' ) + comment + QLatin1Char( ')' ) );
    if ( !email.isEmpty() )
        parts.push_back( QLatin1Char( '<' ) + email + QLatin1Char( '>' ) );

    ui.resultLB->setText( parts.join( QLatin1String( " " ) ) );
}

#include "moc_adduseriddialog.cpp"
