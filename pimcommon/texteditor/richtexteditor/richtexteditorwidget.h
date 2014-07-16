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

#ifndef RICHTEXTEDITORWIDGET_H
#define RICHTEXTEDITORWIDGET_H

#include "pimcommon_export.h"

#include <QWidget>

namespace PimCommon {
class RichTextEditor;
class RichTextEditFindBar;
class PIMCOMMON_EXPORT RichTextEditorWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool readOnly READ isReadOnly WRITE setReadOnly)
public:
    explicit RichTextEditorWidget(QWidget *parent=0);
    explicit RichTextEditorWidget(RichTextEditor *customEditor, QWidget *parent=0);
    ~RichTextEditorWidget();

    void clear();

    RichTextEditor *editor() const { return mEditor; }

    void setReadOnly(bool readOnly);
    bool isReadOnly() const;

    void setHtml(const QString &html);
    QString toHtml() const;

    void setPlainText(const QString &text);
    QString toPlainText() const;

    void setAcceptRichText(bool b);
    bool acceptRichText() const;

private Q_SLOTS:
    void slotFind();
    void slotReplace();

private:
    void init(RichTextEditor *customEditor = 0);
    PimCommon::RichTextEditFindBar *mFindBar;
    RichTextEditor *mEditor;
};
}

#endif // RICHTEXTEDITORWIDGET_H
