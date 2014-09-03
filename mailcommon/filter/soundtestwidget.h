/*
  kmfawidgets.h - KMFilterAction parameter widgets
  Copyright (c) 2001 Marc Mutz <mutz@kde.org>

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
*/

#ifndef MAILCOMMON_SOUNDTESTWIDGET_H
#define MAILCOMMON_SOUNDTESTWIDGET_H

#include <QWidget>

class QPushButton;
class KUrlRequester;

namespace MailCommon
{

/**
 * @short A widget to play a sound from a given URL.
 */
class SoundTestWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * Creates a new sound test widget.
     *
     * @param parent The parent widget.
     */
    explicit SoundTestWidget(QWidget *parent = 0);

    /**
     * Destroys the sound test widget.
     */
    ~SoundTestWidget();

    /**
     * Sets the @p url of the sound file to play.
     */
    void setUrl(const QString &url);

    /**
     * Returns the url of the sound file to play.
     */
    QString url() const;

    /**
     * Clears the url of the sound file to play.
     */
    void clear();

Q_SIGNALS:
    /**
     * This signal is emitted when the user clicked
     * the Play button.
     */
    void testPressed();

    /**
     * This signal is emitted when the user
     * enters a new URL.
     */
    void textChanged(const QString &);

private Q_SLOTS:
    void playSound();
    void openSoundDialog(KUrlRequester *);
    void slotUrlChanged(const QString &);

private:
    KUrlRequester *m_urlRequester;
    QPushButton *m_playButton;
};

}

#endif
