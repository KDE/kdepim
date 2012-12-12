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

    QString html() const;

    void initialize( const QWebElement& element = QWebElement() );

    QWebElement webElement;
    KPIMTextEdit::InsertTableWidget *insertTableWidget;
    ComposerTableDialog *q;
};

QString ComposerTableDialogPrivate::html() const
{
    const int numberOfColumns( insertTableWidget->columns() );
    const int numberRow( insertTableWidget->rows() );

    QString htmlTable = QString::fromLatin1("<table border='%1'").arg(insertTableWidget->border());
    htmlTable += QString::fromLatin1(" width='%1%2'").arg(insertTableWidget->length()).arg(insertTableWidget->typeOfLength() == QTextLength::PercentageLength ? QLatin1String("%") : QString());
    htmlTable += QString::fromLatin1(">");
    for(int i = 0; i <numberRow; ++i) {
        htmlTable += QLatin1String("<tr>");
        for(int j = 0; j <numberOfColumns; ++j) {
            htmlTable += QLatin1String("<td><br></td>");
        }
        htmlTable += QLatin1String("</tr>");
    }
    htmlTable += QLatin1String("</table>");
    return htmlTable;
}

void ComposerTableDialogPrivate::initialize(const QWebElement &element)
{
    webElement = element;
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

ComposerTableDialog::ComposerTableDialog(const QWebElement& element, QWidget *parent)
    : KDialog(parent), d(new ComposerTableDialogPrivate(this))
{
    d->initialize(element);
}

ComposerTableDialog::~ComposerTableDialog()
{
    delete d;
}

QString ComposerTableDialog::html() const
{
    return d->html();
}

}

#include "composertabledialog.moc"
