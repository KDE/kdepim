/*
 * This file is part of libsyndication
 *
 * Copyright (C) 2006 Frank Osterfeld <frank.osterfeld@kdemail.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */
#ifndef LIBSYNDICATION_RDF_RSSVOCAB_H
#define LIBSYNDICATION_RDF_RSSVOCAB_H

template <class T> class KSharedPtr;

class QString;

namespace LibSyndication {
namespace RDF {

class Property;
typedef KSharedPtr<Property> PropertyPtr;

class Resource;
typedef KSharedPtr<Resource> ResourcePtr;
    
class RSSVocab
{
    public:

        virtual ~RSSVocab();
        
        static RSSVocab* self();
        
        const QString& namespaceURI() const;

        PropertyPtr title() const;
        
        PropertyPtr description() const;
        
        PropertyPtr link() const;
        
        PropertyPtr name() const;
        
        PropertyPtr url() const;
        
        ResourcePtr channel() const;
        
        PropertyPtr items() const;
        
        PropertyPtr image() const;
        
        PropertyPtr textinput() const;
        
    protected:
        
        RSSVocab();
        
    private:
        
        static RSSVocab* m_self;
        
        class RSSVocabPrivate;
        RSSVocabPrivate* d;
};

} // namespace RDF
} // namespace LibSyndication

#endif // LIBSYNDICATION_RDF_RSSVOCAB_H
