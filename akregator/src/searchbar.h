/*
    This file is part of Akregator.

    Copyright (C) 2005 Frank Osterfeld <osterfeld@kde.org>

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

#ifndef AKREGATOR_SEARCHBAR_H
#define AKREGATOR_SEARCHBAR_H

#include <khbox.h>

#include <boost/shared_ptr.hpp>
#include <vector>

class QString;

namespace Akregator
{

namespace Filters
{
    class AbstractMatcher;
}

class SearchBar : public KHBox
{
    Q_OBJECT

    public:

        explicit SearchBar(QWidget* parent=0);
        ~SearchBar();

        QString text() const;
        int status() const;

        void setDelay(int ms);
        int delay() const;

        std::vector<boost::shared_ptr<const Filters::AbstractMatcher> > matchers() const;

    signals:
        /** emitted when the text and status filters were updated. Params are textfilter, statusfilter */
        void signalSearch( const std::vector<boost::shared_ptr<const Akregator::Filters::AbstractMatcher> > & );

    public slots:
        void slotClearSearch();
        void slotSetStatus(int status);
        void slotSetText(const QString& text);

    private slots:

        void slotSearchStringChanged(const QString& search);
        void slotSearchComboChanged(int index);
        void slotActivateSearch();

    private:

        class SearchBarPrivate;
        SearchBarPrivate* d;
};

} // namespace Akregator

#endif //AKREGATOR_SEARCHBAR_H
