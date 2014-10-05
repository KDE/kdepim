/*
  Copyright (c) 2012-2013 Montel Laurent <montel@kde.org>

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
#include "extendattributes/extendattributesbutton.h"
#include "utils/composereditorutils_p.h"

#include <QLineEdit>
#include <KLocalizedString>
#include <KSeparator>
#include <KComboBox>

#include <QVBoxLayout>
#include <QLabel>
#include <QWebElement>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QPushButton>

namespace ComposerEditorNG
{
class ComposerLinkDialogPrivate
{
public:
    ComposerLinkDialogPrivate(ComposerLinkDialog *qq)
        : linkText(0),
          linkLocation(0),
          target(0),
          q(qq)
    {
    }

    void initialize(const QWebElement &element = QWebElement());

    QString html() const;

    void updateLinkHtml();
    void fillTarget();
    void updateSettings();

    void _k_slotOkClicked();
    void _k_slotWebElementChanged();

    QWebElement webElement;
    QLineEdit *linkText;
    QLineEdit *linkLocation;
    KComboBox *target;
    ComposerLinkDialog *q;
};

void ComposerLinkDialogPrivate::initialize(const QWebElement &element)
{
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QWidget *mainWidget = new QWidget(q);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    q->setLayout(mainLayout);
    mainLayout->addWidget(mainWidget);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    q->connect(buttonBox, &QDialogButtonBox::accepted, q, &ComposerLinkDialog::accept);
    q->connect(buttonBox, &QDialogButtonBox::rejected, q, &ComposerLinkDialog::reject);
    mainLayout->addWidget(buttonBox);
    webElement = element;

    q->setWindowTitle(webElement.isNull() ? i18n("Create Link") : i18n("Edit Link"));

    QVBoxLayout *vbox = new QVBoxLayout(mainWidget);

    QGridLayout *layout = new QGridLayout;
    vbox->addLayout(layout);

    QLabel *label = new QLabel(i18n("Enter text to display for the link:"));
    layout->addWidget(label, 0, 0);

    linkText = new QLineEdit;
    linkText->setReadOnly(!webElement.isNull());
    linkText->setClearButtonEnabled(true);
    layout->addWidget(linkText, 0, 1);

    label = new QLabel(i18n("Enter the location:"));
    layout->addWidget(label, 1, 0);
    linkLocation = new QLineEdit;
    linkLocation->setClearButtonEnabled(true);
    layout->addWidget(linkLocation, 1, 1);

    label = new QLabel(i18n("Target"));
    layout->addWidget(label, 2, 0);

    target = new KComboBox;
    fillTarget();
    target->setCurrentIndex(0);
    layout->addWidget(target, 2, 1);

    if (!webElement.isNull()) {
        ExtendAttributesButton *button = new ExtendAttributesButton(webElement, ExtendAttributesDialog::Link, q);
        q->connect(button, SIGNAL(webElementChanged()), q, SLOT(_k_slotWebElementChanged()));
        layout->addWidget(button, 3, 1);
    }

    vbox->addWidget(new KSeparator);

    q->connect(q, SIGNAL(clicked()), q, SLOT(_k_slotOkClicked()));
}

void ComposerLinkDialogPrivate::fillTarget()
{
    target->addItem(i18nc("@item:inlistbox Target", "No Set"), QString());
    target->addItem(i18nc("@item:inlistbox Target", "Current Window"), QLatin1String("_self"));
    target->addItem(i18nc("@item:inlistbox Target", "New Window"), QLatin1String("_blank"));
    target->addItem(i18nc("@item:inlistbox Target", "In parent frame"), QLatin1String("_parent"));
    target->addItem(i18nc("@item:inlistbox Target", "In the full body of the window"), QLatin1String("_top"));
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
        QString html = QString::fromLatin1("<a ");
        if (!targetStr.isEmpty()) {
            html += QString::fromLatin1("target=\'%1\'").arg(targetStr);
        }
        html += QString::fromLatin1("href=\'%1\'>%2</a>").arg(url.toString()).arg(linkText->text());
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

ComposerLinkDialog::ComposerLinkDialog(const QString &selectedText, QWidget *parent)
    : QDialog(parent), d(new ComposerLinkDialogPrivate(this))
{
    d->initialize();
    d->linkText->setText(selectedText);
}

ComposerLinkDialog::ComposerLinkDialog(const QWebElement &element, QWidget *parent)
    : QDialog(parent), d(new ComposerLinkDialogPrivate(this))
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

#include "moc_composerlinkdialog.cpp"
