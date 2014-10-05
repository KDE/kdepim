/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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

#include "composeranchordialog.h"
#include "extendattributes/extendattributesbutton.h"
#include "utils/composereditorutils_p.h"

#include <KLocalizedString>
#include <QLineEdit>
#include <KSeparator>

#include <QVBoxLayout>
#include <QLabel>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QPushButton>

namespace ComposerEditorNG
{

class ComposerAnchorDialogPrivate
{
public:
    ComposerAnchorDialogPrivate(ComposerAnchorDialog *qq)
        : q(qq)
    {
    }

    void initialize(const QWebElement &element = QWebElement());

    void updateSettings();
    void updateLinkHtml();

    void _k_slotOkClicked();
    void _k_slotWebElementChanged();

    QString html() const;

    QWebElement webElement;
    QLineEdit *anchorName;
    ComposerAnchorDialog *q;
};

void ComposerAnchorDialogPrivate::initialize(const QWebElement &element)
{
    webElement = element;
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QWidget *mainWidget = new QWidget(q);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    q->setLayout(mainLayout);
    mainLayout->addWidget(mainWidget);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    q->connect(buttonBox, &QDialogButtonBox::accepted, q, &ComposerAnchorDialog::accept);
    q->connect(buttonBox, &QDialogButtonBox::rejected, q, &ComposerAnchorDialog::reject);
    //PORTING SCRIPT: WARNING mainLayout->addWidget(buttonBox) must be last item in layout. Please move it.
    mainLayout->addWidget(buttonBox);

    q->setWindowTitle(webElement.isNull() ? i18n("Create Anchor") : i18n("Edit Anchor"));

    QVBoxLayout *vbox = new QVBoxLayout(mainWidget);

    QGridLayout *layout = new QGridLayout;
    vbox->addLayout(layout);

    QLabel *label = new QLabel(i18n("Enter anchor name:"));
    layout->addWidget(label, 0, 0);

    anchorName = new QLineEdit;
    anchorName->setReadOnly(!webElement.isNull());
    anchorName->setClearButtonEnabled(true);
    layout->addWidget(anchorName, 0, 1);

    if (!webElement.isNull()) {
        ExtendAttributesButton *button = new ExtendAttributesButton(webElement, ExtendAttributesDialog::Link, q);
        q->connect(button, SIGNAL(webElementChanged()), q, SLOT(_k_slotWebElementChanged()));
        layout->addWidget(button, 1, 1);
    }

    vbox->addWidget(new KSeparator);

    q->connect(q, SIGNAL(clicked()), q, SLOT(_k_slotOkClicked()));

}

void ComposerAnchorDialogPrivate::_k_slotWebElementChanged()
{
    updateSettings();
}

void ComposerAnchorDialogPrivate::_k_slotOkClicked()
{
    if (!webElement.isNull()) {
        updateLinkHtml();
    }
    q->accept();
}

void ComposerAnchorDialogPrivate::updateSettings()
{

}

QString ComposerAnchorDialogPrivate::html() const
{
    return QString::fromLatin1("<a name=\"%1\"></a>").arg(anchorName->text());
}

void ComposerAnchorDialogPrivate::updateLinkHtml()
{
    //TODO
}

ComposerAnchorDialog::ComposerAnchorDialog(QWidget *parent)
    : QDialog(parent), d(new ComposerAnchorDialogPrivate(this))
{
    d->initialize();
}

ComposerAnchorDialog::ComposerAnchorDialog(const QWebElement &element, QWidget *parent)
    : QDialog(parent), d(new ComposerAnchorDialogPrivate(this))
{
    d->initialize(element);
    d->updateSettings();
}

ComposerAnchorDialog::~ComposerAnchorDialog()
{
    delete d;
}

QString ComposerAnchorDialog::html() const
{
    return d->html();
}

}

#include "moc_composeranchordialog.cpp"
