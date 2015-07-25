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

SieveTextEditWidget::SieveTextEditWidget(KSieveUi::SieveTextEdit *customTextEdit, QWidget *parent)
    : QWidget(parent)
{
    initialize(customTextEdit);
}

SieveTextEditWidget::SieveTextEditWidget(QWidget *parent)
    : QWidget(parent)
{
    initialize();
}

SieveTextEditWidget::~SieveTextEditWidget()
{

}

void SieveTextEditWidget::initialize(KSieveUi::SieveTextEdit *custom)
{
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setMargin(0);
    setLayout(mainLayout);
    if (custom) {
        mTextEdit = custom;
    } else {
        mTextEdit = new KSieveUi::SieveTextEdit;
    }
    mTextEdit->setObjectName(QStringLiteral("textedit"));
    mTextEdit->setShowHelpMenu(false);
    mainLayout->addWidget(mTextEdit);

    mSliderContainer = new PimCommon::SlideContainer(this);
    mSliderContainer->setObjectName(QStringLiteral("slidercontainer"));
    mFindBar = new PimCommon::PlainTextEditFindBar(mTextEdit, this);
    mFindBar->setObjectName(QStringLiteral("findbar"));
    mFindBar->setHideWhenClose(false);
    connect(mFindBar, &PimCommon::TextEditFindBarBase::hideFindBar, mSliderContainer, &PimCommon::SlideContainer::slideOut);
    mSliderContainer->setContent(mFindBar);
    mainLayout->addWidget(mSliderContainer);
    connect(mTextEdit, &SieveTextEdit::findText, this, &SieveTextEditWidget::slotFind);
    connect(mTextEdit, &SieveTextEdit::replaceText, this, &SieveTextEditWidget::slotReplace);

}

void SieveTextEditWidget::setReadOnly(bool readOnly)
{
    mTextEdit->setReadOnly(readOnly);
}

void SieveTextEditWidget::slotReplace()
{
    mFindBar->showReplace();
    mSliderContainer->slideIn();
    mFindBar->focusAndSetCursor();
}

void SieveTextEditWidget::slotFind()
{
    if (mTextEdit->textCursor().hasSelection()) {
        mFindBar->setText(mTextEdit->textCursor().selectedText());
    }
    mTextEdit->moveCursor(QTextCursor::Start);
    mFindBar->showFind();
    mSliderContainer->slideIn();
    mFindBar->focusAndSetCursor();
}

SieveTextEdit *SieveTextEditWidget::textEdit() const
{
    return mTextEdit;
}
