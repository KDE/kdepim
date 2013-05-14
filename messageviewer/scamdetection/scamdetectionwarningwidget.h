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

#ifndef SCAMDETECTIONWARNINGWIDGET_H
#define SCAMDETECTIONWARNINGWIDGET_H

#include <KMessageWidget>

namespace MessageViewer {
class ScamDetectionWarningWidget : public KMessageWidget
{
    Q_OBJECT
public:
    explicit ScamDetectionWarningWidget(QWidget *parent = 0);
    ~ScamDetectionWarningWidget();

public Q_SLOTS:
    void slotShowWarning();

private Q_SLOTS:
    void slotShowDetails(const QString &content);
    void slotAddToWhiteList();

Q_SIGNALS:
    void showDetails();
    void moveMessageToTrash();
    void messageIsNotAScam();
    void addToWhiteList();

private Q_SLOTS:
    void slotDisableScamDetection();
    void slotMessageIsNotAScam();
};
}

#endif // SCAMDETECTIONWARNINGWIDGET_H
