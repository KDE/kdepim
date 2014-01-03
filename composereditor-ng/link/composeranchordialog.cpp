/*
  Copyright (c) 2013, 2014 Montel Laurent <montel.org>

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
#include <KLineEdit>
#include <KSeparator>

#include <QVBoxLayout>
#include <QLabel>

namespace ComposerEditorNG {

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
    KLineEdit *anchorName;
    ComposerAnchorDialog *q;
};

void ComposerAnchorDialogPrivate::initialize(const QWebElement &element)
{
    webElement = element;
    q->setButtons( KDialog::Ok | KDialog::Cancel );

    q->setCaption( webElement.isNull() ? i18n( "Create Anchor" ) : i18n( "Edit Anchor" ) );

    QVBoxLayout *vbox = new QVBoxLayout(q->mainWidget());

    QGridLayout *layout = new QGridLayout;
    vbox->addLayout(layout);

    QLabel *label = new QLabel(i18n("Enter anchor name:"));
    layout->addWidget( label, 0, 0 );

    anchorName = new KLineEdit;
    anchorName->setReadOnly(!webElement.isNull());
    anchorName->setClearButtonShown(true);
    layout->addWidget( anchorName, 0, 1 );

    if (!webElement.isNull()) {
        ExtendAttributesButton *button = new ExtendAttributesButton(webElement,ExtendAttributesDialog::Link,q);
        q->connect(button, SIGNAL(webElementChanged()), q, SLOT(_k_slotWebElementChanged()));
        layout->addWidget( button, 1, 1 );
    }

    vbox->addWidget( new KSeparator );

    q->connect(q, SIGNAL(okClicked()), q, SLOT(_k_slotOkClicked()));

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
    : KDialog(parent), d(new ComposerAnchorDialogPrivate(this))
{
    d->initialize();
}


ComposerAnchorDialog::ComposerAnchorDialog(const QWebElement &element, QWidget *parent)
    : KDialog(parent), d(new ComposerAnchorDialogPrivate(this))
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
