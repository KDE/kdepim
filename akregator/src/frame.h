/*
    This file is part of Akregator.

    Copyright (C) 2004 Sashmit Bhaduri <smt@vfemail.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef FRAME_H
#define FRAME_H

#include <qobject.h>

namespace KParts
{
    class ReadOnlyPart;
}

namespace KPIM
{
    class ProgressItem;
}

namespace Akregator
{

    class Frame : public QObject
    {
        Q_OBJECT

        public:
            Frame(QObject *parent, KParts::ReadOnlyPart *part, QWidget *w, const QString& tit, bool watchSignals=true);
            virtual ~Frame();

            enum {Idle, Started, Completed, Canceled};

            KParts::ReadOnlyPart *part() const;
            QWidget *widget() const;
            const QString& title() const;
            const QString& caption() const;
            int state() const;
            int progress() const;
            const QString& statusText() const;
            
            /** if set to true, the part is deleted when the frame is deleted. Set to @c false by default */
            void setAutoDeletePart(bool autoDelete);

        public slots:
            void setStarted();
            void setCanceled(const QString &);
            void setCompleted();
            void setState(int);
            void setProgress(int);
            void setCaption(const QString &);
            void setTitle(const QString &);
            void setStatusText(const QString &); 

        signals:
            void captionChanged(const QString &);
            void titleChanged(Frame*, const QString&);
            void started();
            void canceled(const QString &);
            void completed();
            void loadingProgress(int);
            void statusText(const QString &);

        private:
            KParts::ReadOnlyPart *m_part;
            QWidget *m_widget;
            QString m_title;
            QString m_caption;
            int m_state;
            int m_progress;
            QString m_statusText;
            QString m_progressId;
            KPIM::ProgressItem *m_progressItem;
            bool m_autoDeletePart;
    };
}

#endif


// vim: set et ts=4 sts=4 sw=4:
