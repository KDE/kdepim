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

#ifndef SUBJECTLINEEDITWITHAUTOCORRECTION_H
#define SUBJECTLINEEDITWITHAUTOCORRECTION_H

#include "pimcommon_export.h"

#include "pimcommon/spellchecklineedit.h"

namespace PimCommon
{
class AutoCorrection;
class LineEditWithAutoCorrectionPrivate;
class PIMCOMMON_EXPORT LineEditWithAutoCorrection : public PimCommon::SpellCheckLineEdit
{
    Q_OBJECT
public:
    explicit LineEditWithAutoCorrection(QWidget *parent, const QString &configFile);
    ~LineEditWithAutoCorrection();

    PimCommon::AutoCorrection *autocorrection() const;

    void setAutocorrection(PimCommon::AutoCorrection *autocorrect);
    void setAutocorrectionLanguage(const QString &language);

protected:
    void keyPressEvent(QKeyEvent *e) Q_DECL_OVERRIDE;

private:
    LineEditWithAutoCorrectionPrivate *const d;
};
}

#endif // SUBJECTLINEEDITWITHAUTOCORRECTION_H
