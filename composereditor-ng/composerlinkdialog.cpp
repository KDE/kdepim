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

#include "composerlinkdialog.h"
#include "composereditorutil_p.h"

#include <KLineEdit>
#include <KLocale>
#include <KSeparator>
#include <KComboBox>

#include <QVBoxLayout>
#include <QLabel>
#include <QWebElement>

namespace ComposerEditorNG {
class ComposerLinkDialogPrivate
{
public:
    ComposerLinkDialogPrivate(ComposerLinkDialog *qq)
        :q(qq)
    {
    }

    void initialize(const QWebElement& element = QWebElement() );

    QString html() const;

    void updateLinkHtml();
    void fillTarget();

    void _k_slotOkClicked();

    QWebElement webElement;
    KLineEdit *linkText;
    KLineEdit *linkLocation;
    KComboBox *target;
    ComposerLinkDialog *q;
};

void ComposerLinkDialogPrivate::initialize(const QWebElement &element)
{
    q->setButtons( KDialog::Ok | KDialog::Cancel );
    webElement = element;

    q->setCaption( webElement.isNull() ? i18n( "Create Link" ) : i18n( "Edit Link" ) );

    QVBoxLayout *vbox = new QVBoxLayout(q->mainWidget());

    QGridLayout *layout = new QGridLayout;
    vbox->addLayout(layout);

    QLabel *label = new QLabel(i18n("Enter text to display for the link:"));
    layout->addWidget( label, 0, 0 );

    linkText = new KLineEdit;
    linkText->setReadOnly(!webElement.isNull());
    linkText->setClearButtonShown(true);
    layout->addWidget( linkText, 0, 1 );

    label = new QLabel(i18n("Enter the location:"));
    layout->addWidget( label, 1, 0 );
    linkLocation = new KLineEdit;
    linkLocation->setClearButtonShown(true);
    layout->addWidget( linkLocation, 1, 1 );

    label = new QLabel(i18n("Target"));
    layout->addWidget( label, 2, 0 );

    target = new KComboBox;
    fillTarget();
    layout->addWidget( target, 2, 1 );

    KSeparator *sep = new KSeparator;
    vbox->addWidget( sep );


    q->connect(q, SIGNAL(okClicked()), q, SLOT(_k_slotOkClicked()));
}

void ComposerLinkDialogPrivate::fillTarget()
{
    //TODO
}

void ComposerLinkDialogPrivate::_k_slotOkClicked()
{
    if (!webElement.isNull()) {
        updateLinkHtml();
    }
    q->accept();
}


QString ComposerLinkDialogPrivate::html() const
{
    const QUrl url = ComposerEditorNG::Util::guessUrlFromString(linkLocation->text());
    if (url.isValid()){
        const QString html = QString::fromLatin1( "<a href=\'%1\'>%2</a>" ).arg ( url.toString() ).arg ( linkText->text() );
        return html;
    }
    return QString();
}

void ComposerLinkDialogPrivate::updateLinkHtml()
{
    if (linkLocation->text().isEmpty()) {
        webElement.removeAttribute(QLatin1String("href"));
    } else {
        webElement.setAttribute(QLatin1String("href"), linkLocation->text());
    }
}


ComposerLinkDialog::ComposerLinkDialog(const QString& selectedText, QWidget *parent)
    : KDialog(parent), d(new ComposerLinkDialogPrivate(this))
{

    d->initialize();
    d->linkText->setText(selectedText);
}

ComposerLinkDialog::ComposerLinkDialog(const QWebElement& element, QWidget *parent)
    : KDialog(parent),d(new ComposerLinkDialogPrivate(this))
{
    d->initialize(element);
    d->linkLocation->setText(d->webElement.attribute(QLatin1String("href")));
    d->linkText->setText(d->webElement.toInnerXml());
}

ComposerLinkDialog::~ComposerLinkDialog()
{
    delete d;
}

QString ComposerLinkDialog::html() const
{
    return d->html();
}

}

#include "composerlinkdialog.moc"
