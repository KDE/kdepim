/*

  Copyright (c) 2012-2015 Montel Laurent <montel@kde.org>

  The program is free software; you can redistribute it and/or modify it
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

#ifndef TRANSLATORWIDGET_H
#define TRANSLATORWIDGET_H

#include "pimcommon_export.h"
#include "kpimtextedit/plaintexteditor.h"
#include <kio/job.h>
#include "kpimtextedit/plaintexteditorwidget.h"
namespace PimCommon
{

class TranslatorResultTextEdit : public KPIMTextEdit::PlainTextEditor
{
    Q_OBJECT
public:
    explicit TranslatorResultTextEdit(QWidget *parent = Q_NULLPTR);

    void setResultFailed(bool failed);

protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;

private:
    bool mResultFailed;
};

class PIMCOMMON_EXPORT TranslatorTextEdit : public KPIMTextEdit::PlainTextEditor
{
    Q_OBJECT
public:
    explicit TranslatorTextEdit(QWidget *parent = Q_NULLPTR);

Q_SIGNALS:
    void translateText();

protected:
    void dropEvent(QDropEvent *) Q_DECL_OVERRIDE;
};

class PIMCOMMON_EXPORT TranslatorWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TranslatorWidget(QWidget *parent = Q_NULLPTR);
    explicit TranslatorWidget(const QString &text, QWidget *parent = Q_NULLPTR);
    ~TranslatorWidget();

    void setTextToTranslate(const QString &);
    void writeConfig();
    void readConfig();
    void setStandalone(bool b);

public Q_SLOTS:
    void slotTranslate();
    void slotCloseWidget();

private Q_SLOTS:
    void slotFromLanguageChanged(int, bool initialize = false);
    void slotTextChanged();
    void slotInvertLanguage();
    void slotClear();
    void slotTranslateDone();
    void slotTranslateFailed(bool result, const QString &message);
    void slotDebug();
    void slotConfigChanged();

protected:
    bool event(QEvent *e) Q_DECL_OVERRIDE;

Q_SIGNALS:
    void toolsWasClosed();

private:
    void init();
    void initLanguage();
    class TranslatorWidgetPrivate;
    TranslatorWidgetPrivate *const d;
};
}

#endif /* TRANSLATORWIDGET_H */

