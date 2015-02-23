/* Copyright (C) 2010 Torgny Nyblom <nyblom@kde.org>
 * Copyright (C) 2010-2015 Laurent Montel <montel@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "findbarbase.h"

#include <qicon.h>
#include <KLocalizedString>
#include <klineedit.h>
#include <KColorScheme>

#include <QPushButton>
#include <QtCore/QTimer>
#include <QLabel>
#include <QLayout>
#include <QMenu>
#include <QToolButton>
#include <QEvent>
#include <QKeyEvent>

using namespace MessageViewer;

FindBarBase::FindBarBase(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *lay = new QHBoxLayout(this);
    lay->setMargin(2);

    QToolButton *closeBtn = new QToolButton(this);
    closeBtn->setIcon(QIcon::fromTheme(QStringLiteral("dialog-close")));
    closeBtn->setObjectName(QStringLiteral("close"));
    closeBtn->setIconSize(QSize(16, 16));
    closeBtn->setToolTip(i18n("Close"));

#ifndef QT_NO_ACCESSIBILITY
    closeBtn->setAccessibleName(i18n("Close"));
#endif

    closeBtn->setAutoRaise(true);
    lay->addWidget(closeBtn);

    QLabel *label = new QLabel(i18nc("Find text", "F&ind:"), this);
    lay->addWidget(label);

    mSearch = new KLineEdit(this);
    mSearch->setObjectName(QStringLiteral("searchline"));
    mSearch->setToolTip(i18n("Text to search for"));
    mSearch->setClearButtonShown(true);
    label->setBuddy(mSearch);
    lay->addWidget(mSearch);

    mFindNextBtn = new QPushButton(QIcon::fromTheme(QStringLiteral("go-down-search")), i18nc("Find and go to the next search match", "Next"), this);
    mFindNextBtn->setToolTip(i18n("Jump to next match"));
    mFindNextBtn->setObjectName(QStringLiteral("findnext"));
    lay->addWidget(mFindNextBtn);
    mFindNextBtn->setEnabled(false);

    mFindPrevBtn = new QPushButton(QIcon::fromTheme(QStringLiteral("go-up-search")), i18nc("Find and go to the previous search match", "Previous"), this);
    mFindPrevBtn->setToolTip(i18n("Jump to previous match"));
    mFindPrevBtn->setObjectName(QStringLiteral("findprevious"));
    lay->addWidget(mFindPrevBtn);
    mFindPrevBtn->setEnabled(false);

    QPushButton *optionsBtn = new QPushButton(this);
    optionsBtn->setText(i18n("Options"));
    optionsBtn->setToolTip(i18n("Modify search behavior"));
    mOptionsMenu = new QMenu(optionsBtn);
    mCaseSensitiveAct = mOptionsMenu->addAction(i18n("Case sensitive"));
    mCaseSensitiveAct->setCheckable(true);

    optionsBtn->setMenu(mOptionsMenu);
    lay->addWidget(optionsBtn);

    connect(closeBtn, &QToolButton::clicked, this, &FindBarBase::closeBar);
    connect(mFindNextBtn, &QPushButton::clicked, this, &FindBarBase::findNext);
    connect(mFindPrevBtn, &QPushButton::clicked, this, &FindBarBase::findPrev);
    connect(mCaseSensitiveAct, &QAction::toggled, this, &FindBarBase::caseSensitivityChanged);
    connect(mSearch, &KLineEdit::textChanged, this, &FindBarBase::autoSearch);
    connect(mSearch, &KLineEdit::clearButtonClicked, this, &FindBarBase::slotClearSearch);

    mStatus = new QLabel;
    mStatus->setObjectName(QStringLiteral("status"));
    QFontMetrics fm(mStatus->font());
    mNotFoundString = i18n("Phrase not found");
    mStatus->setFixedWidth(fm.width(mNotFoundString));
    lay->addWidget(mStatus);

    setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));
    lay->addStretch();
    hide();
}

FindBarBase::~FindBarBase()
{
}

QMenu *FindBarBase::optionsMenu()
{
    return mOptionsMenu;
}

QString FindBarBase::text() const
{
    return mSearch->text();
}

void FindBarBase::setText(const QString &text)
{
    mSearch->setText(text);
}

void FindBarBase::focusAndSetCursor()
{
    setFocus();
    mStatus->clear();
    mSearch->selectAll();
    mSearch->setFocus();
}

void FindBarBase::slotClearSearch()
{
    clearSelections();
}

void FindBarBase::autoSearch(const QString &str)
{
    const bool isNotEmpty = (!str.isEmpty());
    mFindPrevBtn->setEnabled(isNotEmpty);
    mFindNextBtn->setEnabled(isNotEmpty);
    if (isNotEmpty) {
        QTimer::singleShot(0, this, SLOT(slotSearchText()));
    } else {
        clearSelections();
    }
}

void FindBarBase::slotSearchText(bool backward, bool isAutoSearch)
{
    searchText(backward, isAutoSearch);
}

void FindBarBase::setFoundMatch(bool match)
{
#ifndef QT_NO_STYLE_STYLESHEET
    QString styleSheet;

    if (!mSearch->text().isEmpty()) {
        if (mNegativeBackground.isEmpty()) {
            KStatefulBrush bgBrush(KColorScheme::View, KColorScheme::PositiveBackground);
            mPositiveBackground = QStringLiteral("QLineEdit{ background-color:%1 }").arg(bgBrush.brush(mSearch).color().name());
            bgBrush = KStatefulBrush(KColorScheme::View, KColorScheme::NegativeBackground);
            mNegativeBackground = QStringLiteral("QLineEdit{ background-color:%1 }").arg(bgBrush.brush(mSearch).color().name());
        }
        if (match) {
            styleSheet = mPositiveBackground;
            mStatus->clear();
        } else {
            styleSheet = mNegativeBackground;
            mStatus->setText(mNotFoundString);
        }
    }
    mSearch->setStyleSheet(styleSheet);
#endif

}

void FindBarBase::searchText(bool backward, bool isAutoSearch)
{
    Q_UNUSED(backward);
    Q_UNUSED(isAutoSearch);
}

void FindBarBase::findNext()
{
    searchText(false, false);
}

void FindBarBase::findPrev()
{
    searchText(true, false);
}

void FindBarBase::caseSensitivityChanged(bool b)
{
    updateSensitivity(b);
}

void FindBarBase::updateSensitivity(bool)
{
}

void FindBarBase::slotHighlightAllChanged(bool b)
{
    updateHighLight(b);
}

void FindBarBase::updateHighLight(bool)
{
}

void FindBarBase::clearSelections()
{
    setFoundMatch(false);
}

void FindBarBase::closeBar()
{
    // Make sure that all old searches are cleared
    mSearch->clear();
    clearSelections();
    mSearch->clearFocus();
    Q_EMIT hideFindBar();
}

bool FindBarBase::event(QEvent *e)
{
    // Close the bar when pressing Escape.
    // Not using a QShortcut for this because it could conflict with
    // window-global actions (e.g. Emil Sedgh binds Esc to "close tab").
    // With a shortcut Q_DECL_OVERRIDE we can catch this before it gets to kactions.
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
            if (mSearch->text().isEmpty()) {
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

