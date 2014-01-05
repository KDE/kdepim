/*
  Copyright (c) 2013, 2014 Montel Laurent <montel.org>

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

#ifndef TEST_AUTOCORRECTION_GUI_H
#define TEST_AUTOCORRECTION_GUI_H

#include <QWidget>
#include <KSharedConfig>
#include <KDialog>

#include <QTextEdit>

namespace PimCommon {
class AutoCorrection;
class AutoCorrectionWidget;
class LineEditWithAutoCorrection;
}

class ConfigureTestDialog : public KDialog
{
    Q_OBJECT
public:
    explicit ConfigureTestDialog(PimCommon::AutoCorrection *autoCorrection, QWidget *parent=0);
    ~ConfigureTestDialog();

private Q_SLOTS:
    void slotSaveSettings();

private:
    PimCommon::AutoCorrectionWidget *mWidget;
};

class TextEditAutoCorrectionWidget : public QTextEdit
{
    Q_OBJECT
public:
    explicit TextEditAutoCorrectionWidget(PimCommon::AutoCorrection *autoCorrection, QWidget *parent=0);
    ~TextEditAutoCorrectionWidget();

protected:
    void keyPressEvent ( QKeyEvent *e );

private:
    PimCommon::AutoCorrection *mAutoCorrection;
};

class AutocorrectionTestWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AutocorrectionTestWidget(QWidget *parent=0);
    ~AutocorrectionTestWidget();

private Q_SLOTS:
    void slotConfigure();
    void slotChangeMode(bool);

private:
    TextEditAutoCorrectionWidget *mEdit;
    PimCommon::LineEditWithAutoCorrection *mSubject;
    PimCommon::AutoCorrection *mAutoCorrection;
    KSharedConfig::Ptr mConfig;
};

#endif
