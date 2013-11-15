/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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

#include "subjectlineeditwithautocorrection.h"
#include "autocorrection/autocorrection.h"

#include <QKeyEvent>

using namespace PimCommon;

SubjectLineEditWithAutoCorrection::SubjectLineEditWithAutoCorrection(QWidget* parent, const QString& configFile)
    : KPIM::SpellCheckLineEdit(parent, configFile),
      mAutoCorrection(0)
{
}

SubjectLineEditWithAutoCorrection::~SubjectLineEditWithAutoCorrection()
{

}

AutoCorrection *SubjectLineEditWithAutoCorrection::autocorrection() const
{
    return mAutoCorrection;
}

void SubjectLineEditWithAutoCorrection::setAutocorrection(PimCommon::AutoCorrection* autocorrect)
{
    mAutoCorrection = autocorrect;
}

void SubjectLineEditWithAutoCorrection::setAutocorrectionLanguage(const QString &language)
{
    mAutoCorrection->setLanguage(language);
}


void SubjectLineEditWithAutoCorrection::keyPressEvent ( QKeyEvent *e )
{
    if ((e->key() == Qt::Key_Space) || (e->key() == Qt::Key_Enter) || (e->key() == Qt::Key_Return)) {
        if (mAutoCorrection) {
            // no Html format in subject.
            mAutoCorrection->autocorrect(false, *document(),textCursor().position());
            if (e->key() == Qt::Key_Space) {
                textCursor().insertText(QLatin1String(" "));
                return;
            }
        }
    }
    KPIM::SpellCheckLineEdit::keyPressEvent( e );
}

