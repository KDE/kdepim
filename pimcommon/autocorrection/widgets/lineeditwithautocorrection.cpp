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

#include "autocorrection/widgets/lineeditwithautocorrection.h"
#include "autocorrection/autocorrection.h"

#include <QKeyEvent>

using namespace PimCommon;

LineEditWithAutoCorrection::LineEditWithAutoCorrection(QWidget *parent, const QString &configFile)
    : KPIM::SpellCheckLineEdit(parent, configFile),
      mAutoCorrection(new PimCommon::AutoCorrection()),
      mNeedToDeleteAutoCorrection(true)
{
}

LineEditWithAutoCorrection::~LineEditWithAutoCorrection()
{
    if (mNeedToDeleteAutoCorrection) {
        delete mAutoCorrection;
    }
}

AutoCorrection *LineEditWithAutoCorrection::autocorrection() const
{
    return mAutoCorrection;
}

void LineEditWithAutoCorrection::setAutocorrection(PimCommon::AutoCorrection *autocorrect)
{
    mNeedToDeleteAutoCorrection = false;
    delete mAutoCorrection;
    mAutoCorrection = autocorrect;
}

void LineEditWithAutoCorrection::setAutocorrectionLanguage(const QString &language)
{
    mAutoCorrection->setLanguage(language);
}

void LineEditWithAutoCorrection::keyPressEvent(QKeyEvent *e)
{
    if (mAutoCorrection && mAutoCorrection->isEnabledAutoCorrection()) {
        if ((e->key() == Qt::Key_Space) || (e->key() == Qt::Key_Enter) || (e->key() == Qt::Key_Return)) {
            if (!textCursor().hasSelection()) {
                // no Html format in subject.
                int position = textCursor().position();
                const bool addSpace = mAutoCorrection->autocorrect(false, *document(), position);
                QTextCursor cur = textCursor();
                cur.setPosition(position);
                if (e->key() == Qt::Key_Space) {
                    if (addSpace) {
                        cur.insertText(QLatin1String(" "));
                        setTextCursor(cur);
                    }
                    return;
                }
            }
        }
    }
    KPIM::SpellCheckLineEdit::keyPressEvent(e);
}

