/*
  Copyright (c) 2012 Montel Laurent <montel@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.

*/

#include "composertabledialog.h"

#include <KPIMTextEdit/InsertTableWidget>

#include <KLocale>

#include <QWebElement>

namespace ComposerEditorNG
{
class ComposerTableDialogPrivate
{
public:
    ComposerTableDialogPrivate(ComposerTableDialog *qq)
        :q(qq)
    {
    }

    void initialize( const QWebElement& element = QWebElement() );

    QWebElement webElement;
    KPIMTextEdit::InsertTableWidget *insertTableWidget;
    ComposerTableDialog *q;
};

void ComposerTableDialogPrivate::initialize(const QWebElement &element)
{
    q->setCaption( element.isNull() ? i18n( "Insert Table" ) : i18n( "Edit Table" ) );
    q->setButtons( KDialog::Ok|KDialog::Cancel );
    q->setButtonText( KDialog::Ok, i18n( "Insert" ) );
    insertTableWidget = new KPIMTextEdit::InsertTableWidget( q );
    q->setMainWidget( insertTableWidget );
}

ComposerTableDialog::ComposerTableDialog(QWidget *parent)
    : KDialog(parent), d(new ComposerTableDialogPrivate(this))
{
    d->initialize();
}

ComposerTableDialog::~ComposerTableDialog()
{
    delete d;
}

}
