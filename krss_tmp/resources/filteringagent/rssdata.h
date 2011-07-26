/*
    Copyright (C) 2009    Dmitry Ivanov <vonami@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef RSSFILTERINGAGENT_RSSDATA_H
#define RSSFILTERINGAGENT_RSSDATA_H

#include <akonadi/item.h>
#include <akonadi/filter/data.h>
#include <akonadi/filter/datamemberdescriptor.h>

class RssData : public Akonadi::Filter::Data
{
public:
    explicit RssData( const Akonadi::Item& item );

    bool executeCommand( const Akonadi::Filter::CommandDescriptor* command,
                         const QList<QVariant>& parameters );
    QVariant getPropertyValue( const Akonadi::Filter::FunctionDescriptor* function,
                               const Akonadi::Filter::DataMemberDescriptor* dataMember );

protected:
    QVariant getDataMemberValue( const Akonadi::Filter::DataMemberDescriptor* dataMember );

private:
    Akonadi::Item m_item;
};

#endif // RSSFILTERINGAGENT_RSSDATA_H
