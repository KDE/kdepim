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

#ifndef SIEVEEDITORPARSINGMISSINGFEATUREWARNING_H
#define SIEVEEDITORPARSINGMISSINGFEATUREWARNING_H

#include <KMessageWidget>

namespace KSieveUi {
class SieveEditorParsingMissingFeatureWarning : public KMessageWidget
{
    Q_OBJECT
public:
    enum TextEditorType {
        TextEditor,
        GraphicEditor
    };

    explicit SieveEditorParsingMissingFeatureWarning(SieveEditorParsingMissingFeatureWarning::TextEditorType type, QWidget *parent = 0);
    ~SieveEditorParsingMissingFeatureWarning();

    void setErrors(const QString &initialScript, const QString &errors);
    QString initialScript() const;

Q_SIGNALS:
    void switchToGraphicalMode();
    void switchToTextMode();

private Q_SLOTS:
    void slotSwitchInGraphicalMode();
    void slotSwitchInTextMode();
    void slotInActualMode();
    void slotShowDetails(const QString &content);

private:
    QString mErrors;
    QString mScript;
};
}


#endif // SIEVEEDITORPARSINGMISSINGFEATUREWARNING_H
