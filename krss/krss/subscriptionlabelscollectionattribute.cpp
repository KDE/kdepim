/*
    Copyright (C) 2008    Dmitry Ivanov <vonami@gmail.com>

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

#include "subscriptionlabelscollectionattribute.h"

using namespace KRss;

SubscriptionLabelsCollectionAttribute::SubscriptionLabelsCollectionAttribute( const QStringList &subscriptionLabels )
        : Attribute(), m_subscriptionLabels( subscriptionLabels )
{
}

QByteArray SubscriptionLabelsCollectionAttribute::type() const
{
        return "SubscriptionLabels";
}

SubscriptionLabelsCollectionAttribute* SubscriptionLabelsCollectionAttribute::clone() const
{
        SubscriptionLabelsCollectionAttribute *attr = new SubscriptionLabelsCollectionAttribute( m_subscriptionLabels );
        return attr;
}

QByteArray SubscriptionLabelsCollectionAttribute::serialized() const
{
        return m_subscriptionLabels.join( ";" ).toUtf8();
}

void SubscriptionLabelsCollectionAttribute::deserialize( const QByteArray &data )
{
        if ( data.isEmpty() )
                return;

        // so ugly, am i missing something?
        m_subscriptionLabels = QString::fromUtf8( data.constData(), data.size() ).split( ';' );
}

QStringList SubscriptionLabelsCollectionAttribute::subscriptionLabels() const
{
        return m_subscriptionLabels;
}

void SubscriptionLabelsCollectionAttribute::setSubscriptionLabels( const QStringList &subscriptionLabels )
{
        m_subscriptionLabels = subscriptionLabels;
}

void SubscriptionLabelsCollectionAttribute::addSubscriptionLabel( const QString &subscriptionLabel )
{
        if ( !m_subscriptionLabels.contains( subscriptionLabel ) )
                m_subscriptionLabels << subscriptionLabel;
}

void SubscriptionLabelsCollectionAttribute::removeSubscriptionLabel( const QString &subscriptionLabel )
{
        // addSubscriptionLabel ensures the list contains no duplicates
        m_subscriptionLabels.removeOne( subscriptionLabel );
}
