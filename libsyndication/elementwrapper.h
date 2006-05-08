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
#ifndef LIBSYNDICATION_ELEMENTWRAPPER_H
#define LIBSYNDICATION_ELEMENTWRAPPER_H

#include <QtCore/QString>

#include <libsyndication/sharedptr.h>

#include <kdepim_export.h>

class QDomElement;
template <class T> class QList;

namespace Syndication {

/**
 * A wrapper for XML elements. This is the base class for the (lazy) wrappers
 * used in the RSS2 and Atom parsers. The wrapped element can be accessed
 * via element(). It also contains several helper functions for XML processing.
 *
 * @author Frank Osterfeld
 */
class SYNDICATION_EXPORT ElementWrapper
{
    public:
        
        /** 
         * creates a element wrapper wrapping a null element.
         * isNull() will return @c true for these instances.
         */
        ElementWrapper();
        
        /**
         * Copy constructor.The instances share the same element.
         * @param other the element wrapper to copy
         */
        ElementWrapper(const ElementWrapper& other);
        
        /**
         * Creates an element wrapper wrapping the DOM element @c element
         * @param element the element to wrap
         */
        ElementWrapper(const QDomElement& element);
        
        /**
         * destructor
         */
        virtual ~ElementWrapper();

        /**
         * Assigns another element wrapper to this one. Both instances
         * share the same wrapped element instance.
         * 
         * @param other the element wrapper to assign
         * @return reference to this instance
         */
        virtual ElementWrapper& operator=(const ElementWrapper& other);
        
        /**
         * compares two wrappers. Two wrappers are equal if and only if
         * the wrapped elements are equal.
         * @param other another element wrapper to compare to
         */
        bool operator==(const ElementWrapper& other) const;
        
        /**
         * returns the wrapped resource.
         */
        const QDomElement& element() const;

        /**
         * returns whether the wrapped element is a null element
         * @return @c true if isNull() is true for the wrapped element,
         * @c false otherwise
         */
        bool isNull() const;
        
        /**
         * returns the xml:base value to be used for the wrapped element.
         * The xml:base attribute establishes the base URI for resolving any
         * relative references found in its scope (its own element and all 
         * descendants). (See also completeURI())
         * 
         * @return the xml:base value, or a null string if not set
         */
        QString xmlBase() const;
        
        /**
         * returns the xml:lang value to be used for the wrapped element.
         * The xml:lang attribute indicates the natural language for its element
         * and all descendants.
         * 
         * @return the xml:lang value, or a null string if not set
         */
        QString xmlLang() const;
        
        /**
         * completes relative URIs with a prefix specified via xml:base.
         * 
         * Example: 
         * @code
         * xml:base="http://www.foo.org/", uri="announcements/bar.html"
         * @endcode
         * 
         * is completed to @c http://www.foo.org/announcements/bar.html
         * 
         * See also xmlBase().
         * 
         * @param uri a possibly relative URI
         * @return the resolved, absolute URI (using xml:base), if @c uri is
         * a relative, valid URI. If @c uri is not valid, absolute, or no 
         * xml:base is set in the scope of this element, @c uri is returned
         * unmodified.
         */
        QString completeURI(const QString& uri) const;
        
        /**
         * extracts the text from a child element, respecting namespaces. If
         * there is more than one child with the same tag name, the first one is 
         * processed.
         * For instance, when the wrapped element is @c &lt;hisElement>:
         * @code
         * <thisElement>
         *     <atom:title>Hi there</atom:title>
         * </thisElement>    
         * @endcode
         * @code 
         * extractElementText("http://www.w3.org/2005/Atom", "title") 
         * @endcode 
         * will return the text content of @c atom:title, "Hi there".
         * (Assuming that "atom" is defined as "http://www.w3.org/2005/Atom")
         * 
         * @param namespaceURI the namespace URI of the element to extract
         * @param localName the local name (local within its namespace) of the
         * element to extract
         * @return the (trimmed) text content of @c localName, or a null string
         * if there is no such tag
         */

        QString extractElementTextNS(const QString& namespaceURI, 
                                     const QString& localName) const;
        
        /**
         * extracts the text from a child element, ignoring namespaces. For 
         * instance, when the wrapped element is @c &lt;thisElement>:
         * @code
         * <thisElement>
         *     <title>Hi there</title>
         * </thisElement>    
         * @endcode
         * @c extractElementText("title") will return the text content
         * of @c title, "Hi there".
         * 
         * @param tagName the name of the element to extract
         * @return the (trimmed) text content of @c tagName, or a null string if
         * there is no such tag
         */
        QString extractElementText(const QString& tagName) const;

        /**
         * returns all child elements with tag name @c tagName 
         * Contrary to QDomElement::elementsByTagName() only direct descendents
         * are returned.
         * 
         * @param tagName the tag name of the elements to extract
         * @return a list of child elements with the given tag name
         */
        QList<QDomElement> elementsByTagName(const QString& tagName) const;
    
        /**
         * returns the child nodes of the wrapped element as XML.
         * 
         * See childNodesAsXML(const QDomElement& parent) for details
         * @return XML serialization of the wrapped element's children
         */
        QString childNodesAsXML() const;
        
        /**
         * concatenates the XML representations of all children. Example: If 
         * @c parent is an @c xhtml:body element like
         * @code
         * <xhtml:body><p>foo</p><blockquote>bar</blockquote></xhtml:body>
         * @endcode
         * this function returns
         * @code
         * <p>foo</p><blockquote>bar</blockquote>
         * @endcode
         *
         * namespace and xml:base information are preserved.
         * 
         * @param parent the DOM element whose children should be returned as
         * XML
         * @return XML serialization of parent's children
         */
        static QString childNodesAsXML(const QDomElement& parent);
        
        /**
         * returns all child elements with tag name @c tagname 
         * and namespace URI @c nsURI.
         * Contrary to QDomElement::elementsByTagNameNS() only direct
         * descendents are returned
         * 
         * @param nsURI the namespace URI
         * @param tagName the local name (local within its namespace) of the 
         * element to search for
         * @return a list of child elements with the given namespace URI
         * and tag name
         */
        QList<QDomElement> elementsByTagNameNS(const QString& nsURI, 
                                               const QString& tagName) const;
        
        /**
         * searches the direct children of the wrapped element for an element
         * with a given namespace and tag name.
         * 
         * @param nsURI the namespace URI
         * @param tagName the local name (local within its namespace) of the 
         * element to search for
         * @return the first child element with the given namespace URI and tag
         * name, or a null element if no such element was found.
         */
        QDomElement firstElementByTagNameNS(const QString& nsURI,
                                            const QString& tagName) const;
        
        /** 
         * Returns the wrapped element's text or an empty string.
         * For more information, see QDomElement::text();
         */
        QString text() const;
        
        /** 
         * Returns the attribute called name. If the attribute does not exist 
         * defValue is returned.
         * (which is a null string by default).
         * 
         * @param name tag name
         * @param defValue the default value
         */
        QString attribute(const QString& name, 
                          const QString& defValue=QString()) const;
        
        /**
         * Returns the attribute with the local @c name localName and the 
         * namespace URI @c nsURI.
         * If the attribute does not exist @c defValue is returned (which is a
         * null string by default).
         * 
         * @param nsURI namespace URI
         * @param localName local tag name
         * @param defValue the default value
         */
        QString attributeNS(const QString& nsURI, const QString& localName,
                            const QString& defValue=QString()) const;
        
        
        /**
         * Returns true if this element has an attribute called @c name;
         * otherwise returns @c false.
         * 
         * @param name the attribute name (without namespace)
         */
        bool hasAttribute(const QString& name) const;
        
        /**
         * Returns true if this element has an attribute with the local name
         * localName and the namespace URI nsURI; otherwise returns false.
         * 
         * @param nsURI namespace URI
         * @param localName local attribute name
         */
        bool hasAttributeNS(const QString& nsURI, const QString& localName) const;
        
    private:

        class ElementWrapperPrivate;
        SharedPtr<ElementWrapperPrivate> d;
};

} // namespace Syndication

#endif // LIBSYNDICATION_ELEMENTWRAPPER_H
