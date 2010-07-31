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

#include <tqobject.h>

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
            Frame(TQObject *parent, KParts::ReadOnlyPart *part, TQWidget *w, const TQString& tit, bool watchSignals=true);
            virtual ~Frame();

            enum {Idle, Started, Completed, Canceled};

            KParts::ReadOnlyPart *part() const;
            TQWidget *widget() const;
            const TQString& title() const;
            const TQString& caption() const;
            int state() const;
            int progress() const;
            const TQString& statusText() const;
            
            /** if set to true, the part is deleted when the frame is deleted. Set to @c false by default */
            void setAutoDeletePart(bool autoDelete);

        public slots:
            void setStarted();
            void setCanceled(const TQString &);
            void setCompleted();
            void setState(int);
            void setProgress(int);
            void setCaption(const TQString &);
            void setTitle(const TQString &);
            void setStatusText(const TQString &); 

        signals:
            void captionChanged(const TQString &);
            void titleChanged(Frame*, const TQString&);
            void started();
            void canceled(const TQString &);
            void completed();
            void loadingProgress(int);
            void statusText(const TQString &);

        private:
            KParts::ReadOnlyPart *m_part;
            TQWidget *m_widget;
            TQString m_title;
            TQString m_caption;
            int m_state;
            int m_progress;
            TQString m_statusText;
            TQString m_progressId;
            KPIM::ProgressItem *m_progressItem;
            bool m_autoDeletePart;
    };
}

#endif


// vim: set et ts=4 sts=4 sw=4:
