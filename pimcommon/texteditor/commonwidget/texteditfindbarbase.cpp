/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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

#include "texteditfindbarbase.h"
#include "pimcommon/texteditor/commonwidget/textfindreplacewidget.h"

#include <QIcon>
#include <KLocalizedString>
#include <kmessagebox.h>
#include <klineedit.h>
#include <KColorScheme>

#include <QTimer>
#include <QToolButton>
#include <QEvent>
#include <QKeyEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>

using namespace PimCommon;

TextEditFindBarBase::TextEditFindBarBase(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *topLayout = new QVBoxLayout;
    topLayout->setMargin(0);
    QHBoxLayout *lay = new QHBoxLayout;
    lay->setMargin(2);

    topLayout->addLayout(lay);

    QToolButton *closeBtn = new QToolButton(this);
    closeBtn->setIcon(QIcon::fromTheme(QLatin1String("dialog-close")));
    closeBtn->setIconSize(QSize(16, 16));
    closeBtn->setToolTip(i18n("Close"));

#ifndef QT_NO_ACCESSIBILITY
    closeBtn->setAccessibleName(i18n("Close"));
#endif

    closeBtn->setAutoRaise(true);
    lay->addWidget(closeBtn);

    mFindWidget = new TextFindWidget;
    lay->addWidget(mFindWidget);

    mReplaceWidget = new TextReplaceWidget;
    topLayout->addWidget(mReplaceWidget);
    mReplaceWidget->hide();

    connect(closeBtn, &QToolButton::clicked, this, &TextEditFindBarBase::closeBar);
    connect(mFindWidget, &TextFindWidget::findNext, this, &TextEditFindBarBase::findNext);
    connect(mFindWidget, &TextFindWidget::findPrev, this, &TextEditFindBarBase::findPrev);
    connect(mFindWidget, &TextFindWidget::updateSearchOptions, this, &TextEditFindBarBase::slotUpdateSearchOptions);
    connect(mFindWidget, &TextFindWidget::updateSearchOptions, this, &TextEditFindBarBase::slotUpdateSearchOptions);
    connect(mFindWidget, &TextFindWidget::autoSearch, this, &TextEditFindBarBase::autoSearch);
    connect(mFindWidget, &TextFindWidget::clearSearch, this, &TextEditFindBarBase::slotClearSearch);
    connect(mFindWidget, &TextFindWidget::searchStringEmpty, mReplaceWidget, &TextReplaceWidget::slotSearchStringEmpty);
    connect(mReplaceWidget, &TextReplaceWidget::replaceText, this, &TextEditFindBarBase::slotReplaceText);
    connect(mReplaceWidget, &TextReplaceWidget::replaceAllText, this, &TextEditFindBarBase::slotReplaceAllText);
    setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));
    hide();
    setLayout(topLayout);
}

TextEditFindBarBase::~TextEditFindBarBase()
{
}

void TextEditFindBarBase::showFind()
{
    if (documentIsEmpty()) {
        return;
    }
    mReplaceWidget->slotSearchStringEmpty(mFindWidget->search()->text().isEmpty());
    show();
    if (mReplaceWidget->isVisible()) {
        mReplaceWidget->hide();
        updateGeometry();
    }
}

void TextEditFindBarBase::showReplace()
{
    if (viewIsReadOnly()) {
        return;
    }
    if (documentIsEmpty()) {
        return;
    }
    mReplaceWidget->slotSearchStringEmpty(mFindWidget->search()->text().isEmpty());
    show();
    if (!mReplaceWidget->isVisible()) {
        mReplaceWidget->show();
        updateGeometry();
    }
}

void TextEditFindBarBase::setText(const QString &text)
{
    mFindWidget->search()->setText(text);
}

QString TextEditFindBarBase::text() const
{
    return mFindWidget->search()->text();
}

void TextEditFindBarBase::focusAndSetCursor()
{
    setFocus();
    mFindWidget->search()->selectAll();
    mFindWidget->search()->setFocus();
}

void TextEditFindBarBase::slotClearSearch()
{
    clearSelections();
}

void TextEditFindBarBase::autoSearch(const QString &str)
{
    const bool isNotEmpty = (!str.isEmpty());
    if (isNotEmpty) {
        QTimer::singleShot(0, this, SLOT(slotSearchText()));
    } else {
        clearSelections();
    }
}

void TextEditFindBarBase::messageInfo(bool backward, bool isAutoSearch, bool found)
{
    if (!found && !isAutoSearch) {
        if (backward) {
            Q_EMIT displayMessageIndicator(i18n("End of message reached.\nPhrase '%1' could not be found.", mLastSearchStr));
        } else {
            Q_EMIT displayMessageIndicator(i18n("End of message reached.\nPhrase '%1' could not be found.", mLastSearchStr));
        }
    }
}

bool TextEditFindBarBase::searchText(bool backward, bool isAutoSearch)
{
    mLastSearchStr = mFindWidget->search()->text();
    QTextDocument::FindFlags searchOptions = mFindWidget->searchOptions();
    if (backward) {
        searchOptions |= QTextDocument::FindBackward;
    }

    if (isAutoSearch) {
        autoSearchMoveCursor();
    } else if (!mLastSearchStr.contains(mFindWidget->search()->text(), Qt::CaseSensitive)) {
        clearSelections();
    }

    const bool found = searchInDocument(mLastSearchStr, searchOptions);
    mFindWidget->setFoundMatch(found);
    messageInfo(backward, isAutoSearch, found);
    return found;
}

void TextEditFindBarBase::findNext()
{
    searchText(false, false);
}

void TextEditFindBarBase::findPrev()
{
    searchText(true, false);
}

void TextEditFindBarBase::slotUpdateSearchOptions()
{
    const QTextDocument::FindFlags searchOptions = mFindWidget->searchOptions();
    mLastSearchStr = mFindWidget->search()->text();
    searchInDocument(mLastSearchStr, searchOptions);
}

void TextEditFindBarBase::clearSelections()
{
    mFindWidget->setFoundMatch(false);
}

void TextEditFindBarBase::closeBar()
{
    // Make sure that all old searches are cleared
    mFindWidget->search()->setText(QString());
    mReplaceWidget->replace()->setText(QString());
    clearSelections();
    hide();
}

bool TextEditFindBarBase::event(QEvent *e)
{
    // Close the bar when pressing Escape.
    // Not using a QShortcut for this because it could conflict with
    // window-global actions (e.g. Emil Sedgh binds Esc to "close tab").
    // With a shortcut override we can catch this before it gets to kactions.
    const bool shortCutOverride = (e->type() == QEvent::ShortcutOverride);
    if (shortCutOverride || e->type() == QEvent::KeyPress) {
        QKeyEvent *kev = static_cast<QKeyEvent * >(e);
        if (kev->key() == Qt::Key_Escape) {
            if (shortCutOverride) {
                e->accept();
                return true;
            }
            e->accept();
            closeBar();
            return true;
        } else if (kev->key() == Qt::Key_Enter ||
                   kev->key() == Qt::Key_Return) {
            e->accept();
            if (shortCutOverride) {
                return true;
            }
            if (mFindWidget->search()->text().isEmpty()) {
                return true;
            }

            if (kev->modifiers() & Qt::ShiftModifier) {
                findPrev();
            } else if (kev->modifiers() == Qt::NoModifier) {
                findNext();
            }
            return true;
        }
    }
    return QWidget::event(e);
}

