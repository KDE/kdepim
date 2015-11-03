/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

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

#include "autocorrection/widgets/lineeditwithautocorrection.h"
#include "autocorrection/autocorrection.h"

#include <QKeyEvent>

using namespace PimCommon;
class PimCommon::LineEditWithAutoCorrectionPrivate
{
public:
    LineEditWithAutoCorrectionPrivate()
        : mAutoCorrection(new PimCommon::AutoCorrection()),
          mNeedToDeleteAutoCorrection(true)
    {

    }
    ~LineEditWithAutoCorrectionPrivate()
    {
        if (mNeedToDeleteAutoCorrection) {
            delete mAutoCorrection;
        }
    }

    PimCommon::AutoCorrection *mAutoCorrection;
    bool mNeedToDeleteAutoCorrection;
};

LineEditWithAutoCorrection::LineEditWithAutoCorrection(QWidget *parent, const QString &configFile)
    : PimCommon::SpellCheckLineEdit(parent, configFile),
      d(new PimCommon::LineEditWithAutoCorrectionPrivate)
{
}

LineEditWithAutoCorrection::~LineEditWithAutoCorrection()
{
    delete d;
}

AutoCorrection *LineEditWithAutoCorrection::autocorrection() const
{
    return d->mAutoCorrection;
}

void LineEditWithAutoCorrection::setAutocorrection(PimCommon::AutoCorrection *autocorrect)
{
    d->mNeedToDeleteAutoCorrection = false;
    delete d->mAutoCorrection;
    d->mAutoCorrection = autocorrect;
}

void LineEditWithAutoCorrection::setAutocorrectionLanguage(const QString &language)
{
    d->mAutoCorrection->setLanguage(language);
}

void LineEditWithAutoCorrection::keyPressEvent(QKeyEvent *e)
{
    if (d->mAutoCorrection && d->mAutoCorrection->isEnabledAutoCorrection()) {
        if ((e->key() == Qt::Key_Space) || (e->key() == Qt::Key_Enter) || (e->key() == Qt::Key_Return)) {
            if (!textCursor().hasSelection()) {
                // no Html format in subject.
                int position = textCursor().position();
                const bool addSpace = d->mAutoCorrection->autocorrect(false, *document(), position);
                QTextCursor cur = textCursor();
                cur.setPosition(position);
                if (e->key() == Qt::Key_Space) {
                    if (addSpace) {
                        cur.insertText(QStringLiteral(" "));
                        setTextCursor(cur);
                    }
                    return;
                }
            }
        }
    }
    PimCommon::SpellCheckLineEdit::keyPressEvent(e);
}

