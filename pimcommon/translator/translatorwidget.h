/* -*- mode: C++; c-file-style: "gnu" -*-

  Copyright (c) 2012-2013 Montel Laurent <montel@kde.org>

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
#include "pimcommon/plaintexteditor/plaintexteditor.h"
#include <kio/job.h>
#include <KTextEdit>

namespace PimCommon {

class TranslatorResultTextEdit : public PimCommon::PlainTextEditor
{
    Q_OBJECT
public:
    explicit TranslatorResultTextEdit(QWidget *parent = 0);

    void setResultFailed(bool failed);

protected:
    void paintEvent( QPaintEvent *event );

private:
    bool mResultFailed;
};

class TranslatorTextEdit : public KTextEdit
{
    Q_OBJECT
public:
    explicit TranslatorTextEdit(QWidget *parent = 0);

Q_SIGNALS:
    void translateText();

protected:
    void dropEvent( QDropEvent * );
};

class PIMCOMMON_EXPORT TranslatorWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TranslatorWidget( QWidget* parent = 0 );
    explicit TranslatorWidget( const QString& text, QWidget* parent = 0 );
    ~TranslatorWidget();

    void setTextToTranslate( const QString& );
    void writeConfig();
    void readConfig();

public Q_SLOTS:
    void slotTranslate();
    void slotCloseWidget();

private Q_SLOTS:
    void slotFromLanguageChanged( int, bool initialize = false );
    void slotTextChanged();
    void slotInvertLanguage();
    void slotClear();
    void slotTranslateDone();
    void slotTranslateFailed(bool result, const QString &message);
    void slotDebug();
    void slotConfigChanged();

protected:
    bool event(QEvent* e);

Q_SIGNALS:
    void translatorWasClosed();

private:
    void init();
    void initLanguage();
    class TranslatorWidgetPrivate;
    TranslatorWidgetPrivate *const d;
};
}

#endif /* TRANSLATORWIDGET_H */

