/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>
  based on code from okular PageViewMessage

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

#ifndef TEXTMESSAGEINDICATOR_H
#define TEXTMESSAGEINDICATOR_H

/**
 * @short A widget that displays messages in the top-left corner.
 *
 * This is a widget with thin border and rounded corners that displays a given
 * text along as an icon. It's meant to be used for displaying messages to the
 * user by placing this above other widgets.
 */
#include <QWidget>
#include "pimcommon_export.h"
class QTimer;
namespace PimCommon
{
class PIMCOMMON_EXPORT TextMessageIndicator : public QWidget
{
public:
    explicit TextMessageIndicator(QWidget *parent = 0);

    enum Icon {
        None,
        Info,
        Warning,
        Error
    };

    void display(const QString &message, const QString &details = QString(), Icon icon = None, int durationMs = 4000);

protected:
    bool eventFilter(QObject *obj, QEvent *event);
    void paintEvent(QPaintEvent *e);
    void mousePressEvent(QMouseEvent *e);

private:
    QRect computeTextRect(const QString &message, int extra_width) const;
    void computeSizeAndResize();
    QString mMessage;
    QString mDetails;
    QTimer *mTimer;
    QPixmap mSymbol;
    int mLineSpacing;
};
}

#endif // TEXTMESSAGEINDICATOR_H

