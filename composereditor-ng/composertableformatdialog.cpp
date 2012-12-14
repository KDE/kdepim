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

#include "composertableformatdialog.h"

#include <KPIMTextEdit/InsertTableWidget>

#include <KLocale>

#include <QWebElement>

namespace ComposerEditorNG
{
class ComposerTableFormatDialogPrivate
{
public:
    ComposerTableFormatDialogPrivate(ComposerTableFormatDialog *qq)
        :q(qq)
    {
    }

    void initialize( const QWebElement& element);

    void updateTableHtml();

    QWebElement webElement;
    KPIMTextEdit::InsertTableWidget *insertTableWidget;
    ComposerTableFormatDialog *q;
};

void ComposerTableFormatDialogPrivate::updateTableHtml()
{
    if(!webElement.isNull()) {
        webElement.setAttribute(QLatin1String("border"),QString::number(insertTableWidget->border()));
        const QString width = QString::fromLatin1("%1%2").arg(insertTableWidget->length()).arg(insertTableWidget->typeOfLength() == QTextLength::PercentageLength ? QLatin1String("%") : QString());
        webElement.setAttribute(QLatin1String("width"),width);
        //TODO update column/row
    }
}

void ComposerTableFormatDialogPrivate::initialize(const QWebElement &element)
{
    webElement = element;
    q->setCaption( i18n( "Table Format" ) );
    q->setButtons( KDialog::Ok|KDialog::Cancel );
    q->setButtonText( KDialog::Ok, i18n( "Edit" ) );
    insertTableWidget = new KPIMTextEdit::InsertTableWidget( q );
    q->setMainWidget( insertTableWidget );
    q->connect(q,SIGNAL(okClicked()),q,SLOT(slotOkClicked()));
    if(!webElement.isNull()) {
        if(webElement.hasAttribute(QLatin1String("border"))) {
            insertTableWidget->setBorder(webElement.attribute(QLatin1String("border")).toInt());
        }
        if(webElement.hasAttribute(QLatin1String("width"))) {
            QString width = webElement.attribute(QLatin1String("width"));
            if(width.endsWith(QLatin1Char('%'))) {
                insertTableWidget->setTypeOfLength(QTextLength::PercentageLength);
                width.chop(1);
                insertTableWidget->setLength(width.toInt());
            } else {
                insertTableWidget->setTypeOfLength(QTextLength::FixedLength);
                insertTableWidget->setLength(width.toInt());
            }
        }
    }
}

ComposerTableFormatDialog::ComposerTableFormatDialog(const QWebElement& element, QWidget *parent)
    : KDialog(parent), d(new ComposerTableFormatDialogPrivate(this))
{
    d->initialize(element);
}

ComposerTableFormatDialog::~ComposerTableFormatDialog()
{
    delete d;
}


void ComposerTableFormatDialog::slotOkClicked()
{
    if(!d->webElement.isNull()) {
        d->updateTableHtml();
    }
    accept();
}

}

#include "composertableformatdialog.moc"
