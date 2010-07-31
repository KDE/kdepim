/*
 * tools_p.cpp
 *
 * Copyright (c) 2001, 2002, 2003 Frerich Raabe <raabe@kde.org>
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. For licensing and distribution details, check the
 * accompanying file 'COPYING'.
 */
#include "tools_p.h"

#include <krfcdate.h>
#include <tqdom.h>
#include <kcharsets.h>
#include <tqregexp.h>

namespace RSS {

time_t parseISO8601Date(const TQString &s)
{
    // do some sanity check: 26-12-2004T00:00+00:00 is parsed to epoch+1 in the KRFCDate, which is wrong. So let's check if the date begins with YYYY -fo
    if (s.stripWhiteSpace().left(4).toInt() < 1000)
        return 0; // error

    // FIXME: imho this is done in KRFCDate::parseDateISO8601() automatically, so we could omit it? -fo
	if (s.find('T') != -1)
		return KRFCDate::parseDateISO8601(s);
    else
        return KRFCDate::parseDateISO8601(s + "T12:00:00");
}

TQString childNodesAsXML(const TQDomNode& parent)
{
	TQDomNodeList list = parent.childNodes();
	TQString str;
	TQTextStream ts( &str, IO_WriteOnly );
	for (uint i = 0; i < list.count(); ++i)
		ts << list.item(i);
	return str.stripWhiteSpace();
}

static TQString plainTextToHtml(const TQString& plainText)
{
    TQString str(plainText);
    str.replace("&", "&amp;");
    str.replace("\"", "&quot;");
    str.replace("<", "&lt;");
    //str.replace(">", "&gt;");
    str.replace("\n", "<br/>");
    return str;
}

enum ContentFormat { Text, HTML, XML, Binary };
        
static ContentFormat mapTypeToFormat(const TQString& modep, const TQString& typep,  const TQString& src)
{
    TQString mode = modep.isNull() ? "escaped" : modep;
    TQString type = typep;
    
    //"If neither the type attribute nor the src attribute is provided,
    //Atom Processors MUST behave as though the type attribute were
    //present with a value of "text""
    if (type.isNull() && src.isEmpty())
        type = TQString::fromUtf8("text");

    if (type == TQString::fromUtf8("html")
        || type == TQString::fromUtf8("text/html"))
        return HTML;
    
    if (type == TQString::fromUtf8("text")
        || (type.startsWith(TQString::fromUtf8("text/"), false)
        && !type.startsWith(TQString::fromUtf8("text/xml"), false))
       )
        return Text;
    
    TQStringList xmltypes;
    xmltypes.append(TQString::fromUtf8("xhtml"));
    // XML media types as defined in RFC3023:
    xmltypes.append(TQString::fromUtf8("text/xml"));
    xmltypes.append(TQString::fromUtf8("application/xml"));
    xmltypes.append(TQString::fromUtf8("text/xml-external-parsed-entity"));
    xmltypes.append(TQString::fromUtf8("application/xml-external-parsed-entity"));
    xmltypes.append(TQString::fromUtf8("application/xml-dtd"));
    
    
    if (xmltypes.contains(type)
        || type.endsWith(TQString::fromUtf8("+xml"), false)
        || type.endsWith(TQString::fromUtf8("/xml"), false))
        return XML;
    
    return Binary;
}

static TQString extractAtomContent(const TQDomElement& e)
{
    ContentFormat format = mapTypeToFormat(e.attribute("mode"),
                                           e.attribute("type"),
                                           e.attribute("src"));
    
    switch (format)
    {
        case HTML:
        {
            const bool hasPre = e.text().contains( "<pre>", false ) || e.text().contains( "<pre ", false );
            return KCharsets::resolveEntities( hasPre ? e.text() : e.text().simplifyWhiteSpace() );
        }
        case Text:
            return plainTextToHtml(e.text().stripWhiteSpace());
        case XML:
            return childNodesAsXML(e).simplifyWhiteSpace();
        case Binary:
        default:
            return TQString();
    }
    
    return TQString();
}

TQString extractNode(const TQDomNode &parent, const TQString &elemName, bool isInlined)
{
	TQDomNode node = parent.namedItem(elemName);
	if (node.isNull())
		return TQString::null;

	TQDomElement e = node.toElement();
        TQString result = e.text().stripWhiteSpace(); // let's assume plain text
 
        if (elemName == "content") // we have Atom here
        {
            result = extractAtomContent(e);
        }        
        else // check for HTML; not necessary for Atom:content
        {
            bool hasPre = result.contains("<pre>", false) || result.contains("<pre ", false);
            bool hasHtml = hasPre || result.contains("<");	// FIXME: test if we have html, should be more clever -> regexp
            if(!isInlined && !hasHtml)						// perform nl2br if not a inline elt and it has no html elts
                    result = result = result.replace(TQChar('\n'), "<br />");
            if(!hasPre)										// strip white spaces if no <pre>
                    result = result.simplifyWhiteSpace();
        }
        
        return result.isEmpty() ? TQString::null : result;
}

TQString extractTitle(const TQDomNode & parent)
{
    TQDomNode node = parent.namedItem(TQString::fromLatin1("title"));
    if (node.isNull())
        return TQString::null;

    TQString result = node.toElement().text();

    result = KCharsets::resolveEntities(KCharsets::resolveEntities(result).replace(TQRegExp("<[^>]*>"), "").remove("\\"));
	result = result.simplifyWhiteSpace();

    if (result.isEmpty())
        return TQString::null;

    return result;
}

static void authorFromString(const TQString& strp, TQString& name, TQString& email)
{
    TQString str = strp.stripWhiteSpace();
    if (str.isEmpty())
        return;
    
    // look for something looking like a mail address ( "foo@bar.com", 
    // "<foo@bar.com>") and extract it
    
    TQRegExp remail("<?([^@\\s<]+@[^>\\s]+)>?"); // FIXME: user "proper" regexp,
       // search kmail source for it
    
    int pos = remail.search(str);
    if (pos != -1)
    {
        TQString all = remail.cap(0);
        email = remail.cap(1);
        str.replace(all, ""); // remove mail address
    }
    
    // simplify the rest and use it as name
    
    name = str.simplifyWhiteSpace();
    
    // after removing the email, str might have 
    // the format "(Foo M. Bar)". We cut off 
    // parentheses if there are any. However, if
    // str is of the format "Foo M. Bar (President)",
    // we should not cut anything.

    TQRegExp rename("^\\(([^\\)]*)\\)");
    
    pos = rename.search(name);
    
    if (pos != -1)
    {
        name = rename.cap(1);
    }
    
    name = name.isEmpty() ? TQString() : name;
    email = email.isEmpty() ? TQString() : email;
}

TQString parseItemAuthor(const TQDomElement& element, Format format, Version version)
{
    TQString name;
    TQString email;

    TQDomElement dcCreator = element.namedItem("dc:creator").toElement();
    
    if (!dcCreator.isNull())
         authorFromString(dcCreator.text(), name, email);
    else if (format == AtomFeed)
    {
        TQDomElement atomAuthor = element.namedItem("author").toElement();
        if (atomAuthor.isNull())
            atomAuthor = element.namedItem("atom:author").toElement();
        if (!atomAuthor.isNull())
        {
            TQDomElement atomName = atomAuthor.namedItem("name").toElement();
            if (atomName.isNull())
                atomName = atomAuthor.namedItem("atom:name").toElement();
            name = atomName.text().stripWhiteSpace();
            
            TQDomElement atomEmail = atomAuthor.namedItem("email").toElement();
            if (atomEmail.isNull())
                atomEmail = atomAuthor.namedItem("atom:email").toElement();
            email = atomEmail.text().stripWhiteSpace();
        }
    }
    else if (format == RSSFeed)
    {
        authorFromString(element.namedItem("author").toElement().text(), name, email);
    }
    
    if (name.isNull())
        name = email;
    
    if (!email.isNull())
        return TQString("<a href=\"mailto:%1\">%2</a>").arg(email).arg(name);
    else
        return name;
}

} // namespace RSS

// vim:noet:ts=4
