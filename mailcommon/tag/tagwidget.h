/*
  Copyright (c) 2012 Montel Laurent <montel@kde.org>
  
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
class KFontRequester;
class KIconButton;
class KKeySequenceWidget;
class QCheckBox;
class KActionCollection;

namespace MailCommon {

class MAILCOMMON_EXPORT TagWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TagWidget(const QList<KActionCollection *> &actionCollections, QWidget *parent = 0);
    ~TagWidget();

    MailCommon::Tag::SaveFlags saveFlags() const;
    void recordTagSettings( MailCommon::Tag::Ptr tag);


    KLineEdit *tagNameLineEdit() { return mTagNameLineEdit; }
    QCheckBox *textColorCheck() { return mTextColorCheck; }
    QCheckBox *textFontCheck() { return mTextFontCheck; }
    QCheckBox *backgroundColorCheck() { return mBackgroundColorCheck; }
    QCheckBox *inToolBarCheck() { return mInToolbarCheck; }

    KColorCombo *textColorCombo() { return mTextColorCombo; }
    KColorCombo *backgroundColorCombo() { return mBackgroundColorCombo; }

    KFontRequester *fontRequester() { return mFontRequester; }

    KIconButton *iconButton() { return mIconButton; }

    KKeySequenceWidget *keySequenceWidget() { return mKeySequenceWidget; }

    void setTagTextColor(const QColor& color);
    void setTagBackgroundColor(const QColor& color);
    void setTagTextFont(const QFont& font);

Q_SIGNALS:
    void changed();
    void iconNameChanged(const QString&);

private Q_SLOTS:
    void slotEmitChangeCheck();



private:
    KLineEdit *mTagNameLineEdit;

    QCheckBox *mTextColorCheck, *mBackgroundColorCheck,
    *mTextFontCheck, *mInToolbarCheck;

    KColorCombo *mTextColorCombo, *mBackgroundColorCombo;

    KFontRequester *mFontRequester;

    KIconButton *mIconButton;

    KKeySequenceWidget *mKeySequenceWidget;
};
}

#endif // MAILCOMMON_TAGWIDGET_H
