/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "sievetexteditwidget.h"
#include <QVBoxLayout>
#include "editor/sievetextedit.h"

#include "pimcommon/texteditor/plaintexteditor/plaintexteditfindbar.h"
#include "pimcommon/widgets/slidecontainer.h"
using namespace KSieveUi;

class KSieveUi::SieveTextEditWidgetPrivate
{
public:
    SieveTextEditWidgetPrivate()
        : mTextEdit(Q_NULLPTR),
          mSliderContainer(Q_NULLPTR),
          mFindBar(Q_NULLPTR)
    {

    }
    KSieveUi::SieveTextEdit *mTextEdit;
    PimCommon::SlideContainer *mSliderContainer;
    PimCommon::PlainTextEditFindBar *mFindBar;
};

SieveTextEditWidget::SieveTextEditWidget(KSieveUi::SieveTextEdit *customTextEdit, QWidget *parent)
    : QWidget(parent),
      d(new KSieveUi::SieveTextEditWidgetPrivate)
{
    initialize(customTextEdit);
}

SieveTextEditWidget::SieveTextEditWidget(QWidget *parent)
    : QWidget(parent),
      d(new KSieveUi::SieveTextEditWidgetPrivate)
{
    initialize();
}

SieveTextEditWidget::~SieveTextEditWidget()
{
    delete d;
}

void SieveTextEditWidget::initialize(KSieveUi::SieveTextEdit *custom)
{
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setMargin(0);
    setLayout(mainLayout);
    if (custom) {
        d->mTextEdit = custom;
    } else {
        d->mTextEdit = new KSieveUi::SieveTextEdit;
    }
    d->mTextEdit->setObjectName(QStringLiteral("textedit"));
    d->mTextEdit->setShowHelpMenu(false);
    mainLayout->addWidget(d->mTextEdit);

    d->mSliderContainer = new PimCommon::SlideContainer(this);
    d->mSliderContainer->setObjectName(QStringLiteral("slidercontainer"));
    d->mFindBar = new PimCommon::PlainTextEditFindBar(d->mTextEdit, this);
    d->mFindBar->setObjectName(QStringLiteral("findbar"));
    d->mFindBar->setHideWhenClose(false);
    connect(d->mFindBar, &PimCommon::TextEditFindBarBase::hideFindBar, d->mSliderContainer, &PimCommon::SlideContainer::slideOut);
    d->mSliderContainer->setContent(d->mFindBar);
    mainLayout->addWidget(d->mSliderContainer);
    connect(d->mTextEdit, &SieveTextEdit::findText, this, &SieveTextEditWidget::slotFind);
    connect(d->mTextEdit, &SieveTextEdit::replaceText, this, &SieveTextEditWidget::slotReplace);

}

void SieveTextEditWidget::setReadOnly(bool readOnly)
{
    d->mTextEdit->setReadOnly(readOnly);
}

void SieveTextEditWidget::slotReplace()
{
    d->mFindBar->showReplace();
    d->mSliderContainer->slideIn();
    d->mFindBar->focusAndSetCursor();
}

void SieveTextEditWidget::slotFind()
{
    if (d->mTextEdit->textCursor().hasSelection()) {
        d->mFindBar->setText(d->mTextEdit->textCursor().selectedText());
    }
    d->mTextEdit->moveCursor(QTextCursor::Start);
    d->mFindBar->showFind();
    d->mSliderContainer->slideIn();
    d->mFindBar->focusAndSetCursor();
}

SieveTextEdit *SieveTextEditWidget::textEdit() const
{
    return d->mTextEdit;
}
