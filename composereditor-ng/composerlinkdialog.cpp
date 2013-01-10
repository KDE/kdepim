/*
  Copyright (c) 2012-2013 Montel Laurent <montel.org>

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
#include "extendattributesbutton.h"
#include "composereditorutils_p.h"

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
    void updateSettings();

    void _k_slotOkClicked();
    void _k_slotWebElementChanged();

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
    target->setCurrentIndex(0);
    layout->addWidget( target, 2, 1 );

    if (!webElement.isNull()) {
        ExtendAttributesButton *button = new ExtendAttributesButton(webElement,ExtendAttributesDialog::Link,q);
        q->connect(button, SIGNAL(webElementChanged()), q, SLOT(_k_slotWebElementChanged()));
        layout->addWidget( button, 3, 1 );
    }

    KSeparator *sep = new KSeparator;
    vbox->addWidget( sep );


    q->connect(q, SIGNAL(okClicked()), q, SLOT(_k_slotOkClicked()));
}

void ComposerLinkDialogPrivate::fillTarget()
{
    target->addItem(i18n("No Set"), QString());
    target->addItem(i18n("Current Window"), QLatin1String("_self"));
    target->addItem(i18n("New Window"), QLatin1String("_blank"));
    target->addItem(i18n("In parent frame"), QLatin1String("_parent"));
    target->addItem(i18n("In the full body of the window"), QLatin1String("_top"));
}

void ComposerLinkDialogPrivate::_k_slotWebElementChanged()
{
    updateSettings();
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
    const QUrl url = ComposerEditorNG::Utils::guessUrlFromString(linkLocation->text());
    if (url.isValid()) {
        const QString targetStr = target->itemData(target->currentIndex()).toString();
        QString html = QString::fromLatin1( "<a ");
        if(!targetStr.isEmpty()) {
            html += QString::fromLatin1("target=\'%1\'").arg(targetStr);
        }
        html += QString::fromLatin1( "href=\'%1\'>%2</a>" ).arg ( url.toString() ).arg ( linkText->text() );
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
    const QString targetStr = target->itemData(target->currentIndex()).toString();
    if (targetStr.isEmpty()) {
        webElement.removeAttribute(QLatin1String("target"));
    } else {
        webElement.setAttribute(QLatin1String("target"), targetStr);
    }
}

void ComposerLinkDialogPrivate::updateSettings()
{
    if (!webElement.isNull()) {
        linkLocation->setText(webElement.attribute(QLatin1String("href")));
        linkText->setText(webElement.toInnerXml());
        if (webElement.hasAttribute(QLatin1String("target"))) {
            const QString targetStr = webElement.attribute(QLatin1String("target"));
            const int index = target->findData(targetStr);
            if (index > -1) {
                target->setCurrentIndex(index);
            }
        }
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
    d->updateSettings();
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
