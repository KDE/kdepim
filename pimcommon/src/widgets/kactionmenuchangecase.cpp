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

#include "kactionmenuchangecase.h"
#include <KLocalizedString>
#include <KActionCollection>
#include <QAction>
using namespace PimCommon;
class PimCommon::KActionMenuChangeCasePrivate
{
public:
    KActionMenuChangeCasePrivate()
        : mUpperCase(Q_NULLPTR),
          mSentenceCase(Q_NULLPTR),
          mLowerCase(Q_NULLPTR),
          mReverseCase(Q_NULLPTR)
    {

    }

    QAction *mUpperCase;
    QAction *mSentenceCase;
    QAction *mLowerCase;
    QAction *mReverseCase;
};

KActionMenuChangeCase::KActionMenuChangeCase(QObject *parent)
    : KActionMenu(parent),
      d(new PimCommon::KActionMenuChangeCasePrivate)
{
    setText(i18n("Change Case"));
    d->mUpperCase = new QAction(i18n("Uppercase"), this);
    connect(d->mUpperCase, &QAction::triggered, this, &KActionMenuChangeCase::upperCase);

    d->mSentenceCase = new QAction(i18n("Sentence case"), this);
    connect(d->mSentenceCase, &QAction::triggered, this, &KActionMenuChangeCase::sentenceCase);

    d->mLowerCase = new QAction(i18n("Lowercase"), this);
    connect(d->mLowerCase, &QAction::triggered, this, &KActionMenuChangeCase::lowerCase);

    d->mReverseCase = new QAction(i18n("Reverse Case"), this);
    connect(d->mReverseCase, &QAction::triggered, this, &KActionMenuChangeCase::reverseCase);

    addAction(d->mUpperCase);
    addAction(d->mLowerCase);
    addAction(d->mSentenceCase);
    addAction(d->mReverseCase);
}

KActionMenuChangeCase::~KActionMenuChangeCase()
{
    delete d;
}

QAction *KActionMenuChangeCase::upperCaseAction() const
{
    return d->mUpperCase;
}

QAction *KActionMenuChangeCase::sentenceCaseAction() const
{
    return d->mSentenceCase;
}

QAction *KActionMenuChangeCase::lowerCaseAction() const
{
    return d->mLowerCase;
}

QAction *KActionMenuChangeCase::reverseCaseAction() const
{
    return d->mReverseCase;
}

void KActionMenuChangeCase::appendInActionCollection(KActionCollection *ac)
{
    if (ac) {
        ac->addAction(QStringLiteral("change_to_uppercase"), d->mUpperCase);
        ac->addAction(QStringLiteral("change_to_sentencecase"), d->mSentenceCase);
        ac->addAction(QStringLiteral("change_to_lowercase"), d->mLowerCase);
        ac->addAction(QStringLiteral("change_to_reversecase"), d->mReverseCase);
    }
}
