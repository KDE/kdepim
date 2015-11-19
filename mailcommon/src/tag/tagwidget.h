/*
  Copyright (c) 2012-2015 Montel Laurent <montel@kde.org>

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

#ifndef MAILCOMMON_TAGWIDGET_H
#define MAILCOMMON_TAGWIDGET_H

#include "mailcommon_export.h"

#include "tag.h"

#include <QWidget>

class KLineEdit;
class KColorCombo;
class KIconButton;
class KKeySequenceWidget;
class QCheckBox;
class KActionCollection;

namespace MailCommon
{
class TagWidgetPrivate;
class MAILCOMMON_EXPORT TagWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TagWidget(const QList<KActionCollection *> &actionCollections, QWidget *parent = Q_NULLPTR);
    ~TagWidget();

    void recordTagSettings(MailCommon::Tag::Ptr tag);

    KLineEdit *tagNameLineEdit() const;
    QCheckBox *textColorCheck() const;
    QCheckBox *textFontCheck() const;
    QCheckBox *backgroundColorCheck() const;
    QCheckBox *inToolBarCheck() const;

    KColorCombo *textColorCombo() const;
    KColorCombo *backgroundColorCombo() const;

    QCheckBox *textBoldCheck() const;
    QCheckBox *textItalicCheck() const;

    KIconButton *iconButton() const;

    KKeySequenceWidget *keySequenceWidget() const;

    void setTagTextColor(const QColor &color);
    void setTagBackgroundColor(const QColor &color);
    void setTagTextFormat(bool bold, bool italic);

Q_SIGNALS:
    void changed();
    void iconNameChanged(const QString &);

private Q_SLOTS:
    void slotEmitChangeCheck();

private:
    TagWidgetPrivate *const d;
};
}

#endif // MAILCOMMON_TAGWIDGET_H
