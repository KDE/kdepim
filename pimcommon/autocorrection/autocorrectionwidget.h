/*
  Copyright (c) 2012-2013 Montel Laurent <montel@kde.org>

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

#ifndef AutoCorrectionWidget_H
#define AutoCorrectionWidget_H

#include "pimcommon_export.h"
#include "autocorrection/autocorrection.h"
#include <QWidget>

class QTreeWidgetItem;

namespace Ui
{
class AutoCorrectionWidget;
}

namespace PimCommon
{

class PIMCOMMON_EXPORT AutoCorrectionWidget : public QWidget
{
    Q_OBJECT

public:
    enum ImportFileType {
        LibreOffice,
        KMail
    };

    explicit AutoCorrectionWidget(QWidget *parent = 0);
    ~AutoCorrectionWidget();
    void setAutoCorrection(AutoCorrection *autoCorrect);
    void loadConfig();
    void writeConfig();
    void resetToDefault();

private Q_SLOTS:
    /* tab 2 */
    void enableSingleQuotes(bool state);
    void enableDoubleQuotes(bool state);
    void selectSingleQuoteCharOpen();
    void selectSingleQuoteCharClose();
    void setDefaultSingleQuotes();
    void selectDoubleQuoteCharOpen();
    void selectDoubleQuoteCharClose();
    void setDefaultDoubleQuotes();

    /* tab 3 */
    void enableAdvAutocorrection(bool state);
    void addAutocorrectEntry();
    void removeAutocorrectEntry();
    void setFindReplaceText(QTreeWidgetItem *, int);
    void enableAddRemoveButton();

    /* tab 4 */
    void abbreviationChanged(const QString &text);
    void twoUpperLetterChanged(const QString &text);
    void addAbbreviationEntry();
    void removeAbbreviationEntry();
    void addTwoUpperLetterEntry();
    void removeTwoUpperLetterEntry();

    void slotEnableDisableAbreviationList();
    void slotEnableDisableTwoUpperEntry();

    void slotImportAutoCorrection(QAction *act);

    void changeLanguage(int);
    void updateAddRemoveButton();

    void slotExportAutoCorrection();

Q_SIGNALS:
    void changed();

private:
    void emitChanged();
    void addAutoCorrectEntries();
    void loadAutoCorrectionAndException();
    void loadGlobalAutoCorrectionAndException();
    void setLanguage(const QString &lang);
    AutoCorrection::TypographicQuotes m_singleQuotes;
    AutoCorrection::TypographicQuotes m_doubleQuotes;
    QSet<QString> m_upperCaseExceptions;
    QSet<QString> m_twoUpperLetterExceptions;
    QHash<QString, QString> m_autocorrectEntries;
    Ui::AutoCorrectionWidget *ui;
    AutoCorrection *mAutoCorrection;
    bool mWasChanged;
};

}

#endif // AutoCorrectionWidget_H
