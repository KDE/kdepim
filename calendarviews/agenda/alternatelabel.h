/*
  Copyright (c) 2000,2001,2003 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/
#ifndef EVENTVIEWS_ALTERNATELABEL_H
#define EVENTVIEWS_ALTERNATELABEL_H

#include "eventviews_export.h"

#include <QLabel>

namespace EventViews
{

class EVENTVIEWS_EXPORT AlternateLabel : public QLabel
{
    Q_OBJECT
public:
    AlternateLabel(const QString &shortlabel, const QString &longlabel,
                   const QString &extensivelabel = QString(), QWidget *parent = 0);
    ~AlternateLabel();

    virtual QSize minimumSizeHint() const;

    enum TextType {
        Short = 0,
        Long = 1,
        Extensive = 2
    };

    TextType largestFittingTextType() const;
    void setFixedType(TextType type);

public slots:
    void useShortText();
    void useLongText();
    void useExtensiveText();
    void useDefaultText();

protected:
    void resizeEvent(QResizeEvent *) Q_DECL_OVERRIDE;
    virtual void squeezeTextToLabel();
    bool mTextTypeFixed;
    QString mShortText, mLongText, mExtensiveText;
};

}

#endif
