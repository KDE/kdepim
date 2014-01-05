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

#ifndef SIEVEEDITORLOADPROGRESSINDICATOR_H
#define SIEVEEDITORLOADPROGRESSINDICATOR_H

#include <QObject>
#include <KPixmapSequence>
#include <KIcon>

namespace KSieveUi {
class SieveEditorLoadProgressIndicator : public QObject
{
    Q_OBJECT
public:
    explicit SieveEditorLoadProgressIndicator(QObject *parent=0);
    ~SieveEditorLoadProgressIndicator();

    void startAnimation();
    void stopAnimation(bool success);

Q_SIGNALS:
    void pixmapChanged(const QPixmap &);
    void loadFinished(bool success);

private Q_SLOTS:
    void slotTimerDone();

private:
    int mProgressCount;
    KPixmapSequence mProgressPix;
    QTimer *mProgressTimer;
};
}

#endif // SIEVEEDITORLOADPROGRESSINDICATOR_H
