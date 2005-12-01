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

#ifndef AKREGATOR_BROWSERRUN_H 
#define AKREGATOR_BROWSERRUN_H

#include <kparts/browserrun.h>

namespace Akregator
{

class Frame;
    
class BrowserRun : public KParts::BrowserRun
{
	Q_OBJECT
    public:
        BrowserRun(Frame* frame, QWidget* widget, const KURL& url, const KParts::URLArgs&);
        virtual ~BrowserRun();

    signals:

        void signalFoundMimeType(Frame*, const KURL&, const KParts::URLArgs&, const QString&);

    protected:
	    virtual void foundMimeType(const QString& type);

    private slots:
        void killMyself();

    private:
        Frame* m_frame;
        KParts::URLArgs m_args;
};

} // namespace Akregator

#endif // AKREGATOR_BROWSERRUN_H

// vim: set et ts=4 sts=4 sw=4:
