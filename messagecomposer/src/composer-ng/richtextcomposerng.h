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

#ifndef RICHTEXTCOMPOSERNG_H
#define RICHTEXTCOMPOSERNG_H

#include "messagecomposer_export.h"
#include <kpimtextedit/richtextcomposer.h>
#include <KIdentityManagement/Signature>

namespace PimCommon
{
class AutoCorrection;
}

namespace MessageComposer
{
class TextPart;
class RichTextComposerSignatures;
class RichTextComposerNgPrivate;
class MESSAGECOMPOSER_EXPORT RichTextComposerNg : public KPIMTextEdit::RichTextComposer
{
    Q_OBJECT
public:
    explicit RichTextComposerNg(QWidget *parent = Q_NULLPTR);
    ~RichTextComposerNg();

    PimCommon::AutoCorrection *autocorrection() const;
    void setAutocorrection(PimCommon::AutoCorrection *autocorrect);
    void setAutocorrectionLanguage(const QString &lang);

    void fillComposerTextPart(MessageComposer::TextPart *textPart);
    MessageComposer::RichTextComposerSignatures *composerSignature() const;

    void insertSignature(const KIdentityManagement::Signature &signature, KIdentityManagement::Signature::Placement placement, KIdentityManagement::Signature::AddedText addedText);
    QString toCleanHtml() const;

private:
    bool processAutoCorrection(QKeyEvent *event) Q_DECL_OVERRIDE;
    RichTextComposerNgPrivate *const d;
};
}
#endif // RICHTEXTCOMPOSERNG_H
