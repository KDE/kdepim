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
KActionMenuChangeCase::KActionMenuChangeCase(QObject *parent)
    : KActionMenu(parent)
{
    setText(i18n("Change Case"));
    mUpperCase = new QAction( i18n("Uppercase"), this );
    connect( mUpperCase, &QAction::triggered, this, &KActionMenuChangeCase::upperCase );

    mSentenceCase = new QAction( i18n("Sentence case"), this );
    connect( mSentenceCase, &QAction::triggered, this, &KActionMenuChangeCase::sentenceCase );

    mLowerCase = new QAction( i18n("Lowercase"), this );
    connect( mLowerCase, &QAction::triggered, this, &KActionMenuChangeCase::lowerCase );

    mReverseCase = new QAction( i18n("Reverse Case"), this );
    connect( mReverseCase, &QAction::triggered, this, &KActionMenuChangeCase::reverseCase );


    addAction(mUpperCase);
    addAction(mLowerCase);
    addAction(mSentenceCase);
    addAction(mReverseCase);
}

KActionMenuChangeCase::~KActionMenuChangeCase()
{

}

QAction *KActionMenuChangeCase::upperCaseAction() const
{
    return mUpperCase;
}

QAction *KActionMenuChangeCase::sentenceCaseAction() const
{
    return mSentenceCase;
}

QAction *KActionMenuChangeCase::lowerCaseAction() const
{
    return mLowerCase;
}

QAction *KActionMenuChangeCase::reverseCaseAction() const
{
    return mReverseCase;
}

void KActionMenuChangeCase::appendInActionCollection(KActionCollection *ac)
{
    if (ac) {
        ac->addAction( QLatin1String("change_to_uppercase"), mUpperCase );
        ac->addAction( QLatin1String("change_to_sentencecase"), mSentenceCase );
        ac->addAction( QLatin1String("change_to_lowercase"), mLowerCase );
        ac->addAction( QLatin1String("change_to_reversecase"), mReverseCase );
    }
}
