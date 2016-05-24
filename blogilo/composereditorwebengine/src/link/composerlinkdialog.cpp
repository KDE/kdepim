/*
  Copyright (c) 2012-2016 Montel Laurent <montel@kde.org>

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
//#include "extendattributes/extendattributesbutton.h"
#include "utils/composereditorutils_p.h"

#include <QLineEdit>
#include <KLocalizedString>
#include <KSeparator>
#include <KComboBox>

#include <QVBoxLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <QPushButton>

namespace ComposerEditorWebEngine
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

    void initialize();

    QString html() const;

    void updateLinkHtml();
    void fillTarget();
    void updateSettings();

    void _k_slotOkClicked();
    void _k_slotWebElementChanged();

    QLineEdit *linkText;
    QLineEdit *linkLocation;
    KComboBox *target;
    ComposerLinkDialog *q;
};

void ComposerLinkDialogPrivate::initialize()
{
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QWidget *mainWidget = new QWidget(q);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    q->setLayout(mainLayout);
    mainLayout->addWidget(mainWidget);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    q->connect(buttonBox, SIGNAL(accepted()), q, SLOT(_k_slotOkClicked()));
    q->connect(buttonBox, &QDialogButtonBox::rejected, q, &ComposerLinkDialog::reject);
    mainLayout->addWidget(buttonBox);

    q->setWindowTitle(i18n("Create Link"));

    QVBoxLayout *vbox = new QVBoxLayout(mainWidget);

    QGridLayout *layout = new QGridLayout;
    vbox->addLayout(layout);

    QLabel *label = new QLabel(i18n("Enter text to display for the link:"));
    layout->addWidget(label, 0, 0);

    linkText = new QLineEdit;
    //linkText->setReadOnly(!webElement.isNull());
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
#if 0
    if (!webElement.isNull()) {
        ExtendAttributesButton *button = new ExtendAttributesButton(webElement, ExtendAttributesDialog::Link, q);
        q->connect(button, SIGNAL(webElementChanged()), q, SLOT(_k_slotWebElementChanged()));
        layout->addWidget(button, 3, 1);
    }
#endif

    vbox->addWidget(new KSeparator);
}

void ComposerLinkDialogPrivate::fillTarget()
{
    target->addItem(i18nc("@item:inlistbox Target", "No Set"), QString());
    target->addItem(i18nc("@item:inlistbox Target", "Current Window"), QStringLiteral("_self"));
    target->addItem(i18nc("@item:inlistbox Target", "New Window"), QStringLiteral("_blank"));
    target->addItem(i18nc("@item:inlistbox Target", "In parent frame"), QStringLiteral("_parent"));
    target->addItem(i18nc("@item:inlistbox Target", "In the full body of the window"), QStringLiteral("_top"));
}

void ComposerLinkDialogPrivate::_k_slotWebElementChanged()
{
    updateSettings();
}

void ComposerLinkDialogPrivate::_k_slotOkClicked()
{
#if 0
    if (!webElement.isNull()) {
        updateLinkHtml();
    }
#endif
    q->accept();
}

QString ComposerLinkDialogPrivate::html() const
{
    const QUrl url = ComposerEditorWebEngine::Utils::guessUrlFromString(linkLocation->text());
    if (url.isValid()) {
        const QString targetStr = target->itemData(target->currentIndex()).toString();
        QString html = QStringLiteral("<a ");
        if (!targetStr.isEmpty()) {
            html += QStringLiteral("target=\'%1\'").arg(targetStr);
        }
        html += QStringLiteral("href=\'%1\'>%2</a>").arg(url.toString()).arg(linkText->text());
        return html;
    }
    return QString();
}

void ComposerLinkDialogPrivate::updateLinkHtml()
{
#if 0
    if (linkLocation->text().isEmpty()) {
        webElement.removeAttribute(QStringLiteral("href"));
    } else {
        webElement.setAttribute(QStringLiteral("href"), linkLocation->text());
    }
    const QString targetStr = target->itemData(target->currentIndex()).toString();
    if (targetStr.isEmpty()) {
        webElement.removeAttribute(QStringLiteral("target"));
    } else {
        webElement.setAttribute(QStringLiteral("target"), targetStr);
    }
#endif
}

void ComposerLinkDialogPrivate::updateSettings()
{
#if 0
    if (!webElement.isNull()) {
        linkLocation->setText(webElement.attribute(QStringLiteral("href")));
        linkText->setText(webElement.toInnerXml());
        if (webElement.hasAttribute(QStringLiteral("target"))) {
            const QString targetStr = webElement.attribute(QStringLiteral("target"));
            const int index = target->findData(targetStr);
            if (index > -1) {
                target->setCurrentIndex(index);
            }
        }
    }
#endif
}

ComposerLinkDialog::ComposerLinkDialog(const QString &selectedText, QWidget *parent)
    : QDialog(parent), d(new ComposerLinkDialogPrivate(this))
{
    d->initialize();
    d->linkText->setText(selectedText);
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
