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
#include <qdom.h>
#include <kcharsets.h>
#include <qregexp.h>

namespace RSS {

time_t parseISO8601Date(const QString &s)
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

QString childNodesAsXML(const QDomNode& parent)
{
	QDomNodeList list = parent.childNodes();
	QString str;
	QTextStream ts( &str, IO_WriteOnly );
	for (uint i = 0; i < list.count(); ++i)
		ts << list.item(i);
	return str.stripWhiteSpace();
}

static QString plainTextToHtml(const QString& plainText)
{
    QString str(plainText);
    str.replace("&", "&amp;");
    str.replace("\"", "&quot;");
    str.replace("<", "&lt;");
    //str.replace(">", "&gt;");
    str.replace("\n", "<br/>");
    return str;
}

enum ContentFormat { Text, HTML, XML, Binary };
        
static ContentFormat mapTypeToFormat(const QString& modep, const QString& typep,  const QString& src)
{
    QString mode = modep.isNull() ? "escaped" : modep;
    QString type = typep;
    
    //"If neither the type attribute nor the src attribute is provided,
    //Atom Processors MUST behave as though the type attribute were
    //present with a value of "text""
    if (type.isNull() && src.isEmpty())
        type = QString::fromUtf8("text");

    if (type == QString::fromUtf8("html")
        || type == QString::fromUtf8("text/html"))
        return HTML;
    
    if (type == QString::fromUtf8("text")
        || (type.startsWith(QString::fromUtf8("text/"), false)
        && !type.startsWith(QString::fromUtf8("text/xml"), false))
       )
        return Text;
    
    QStringList xmltypes;
    xmltypes.append(QString::fromUtf8("xhtml"));
    // XML media types as defined in RFC3023:
    xmltypes.append(QString::fromUtf8("text/xml"));
    xmltypes.append(QString::fromUtf8("application/xml"));
    xmltypes.append(QString::fromUtf8("text/xml-external-parsed-entity"));
    xmltypes.append(QString::fromUtf8("application/xml-external-parsed-entity"));
    xmltypes.append(QString::fromUtf8("application/xml-dtd"));
    
    
    if (xmltypes.contains(type)
        || type.endsWith(QString::fromUtf8("+xml"), false)
        || type.endsWith(QString::fromUtf8("/xml"), false))
        return XML;
    
    return Binary;
}

static QString extractAtomContent(const QDomElement& e)
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
            return QString();
    }
    
    return QString();
}

QString extractNode(const QDomNode &parent, const QString &elemName, bool isInlined)
{
	QDomNode node = parent.namedItem(elemName);
	if (node.isNull())
		return QString::null;

	QDomElement e = node.toElement();
        QString result = e.text().stripWhiteSpace(); // let's assume plain text
 
        if (elemName == "content") // we have Atom here
        {
            result = extractAtomContent(e);
        }        
        else // check for HTML; not necessary for Atom:content
        {
            bool hasPre = result.contains("<pre>", false) || result.contains("<pre ", false);
            bool hasHtml = hasPre || result.contains("<");	// FIXME: test if we have html, should be more clever -> regexp
            if(!isInlined && !hasHtml)						// perform nl2br if not a inline elt and it has no html elts
                    result = result = result.replace(QChar('\n'), "<br />");
            if(!hasPre)										// strip white spaces if no <pre>
                    result = result.simplifyWhiteSpace();
        }
        
        return result.isEmpty() ? QString::null : result;
}

QString extractTitle(const QDomNode & parent)
{
    QDomNode node = parent.namedItem(QString::fromLatin1("title"));
    if (node.isNull())
        return QString::null;

    QString result = node.toElement().text();

    result = KCharsets::resolveEntities(KCharsets::resolveEntities(result).replace(QRegExp("<[^>]*>"), "").remove("\\"));
	result = result.simplifyWhiteSpace();

    if (result.isEmpty())
        return QString::null;

    return result;
}

static void authorFromString(const QString& strp, QString& name, QString& email)
{
    QString str = strp.stripWhiteSpace();
    if (str.isEmpty())
        return;
    
    // look for something looking like a mail address ( "foo@bar.com", 
    // "<foo@bar.com>") and extract it
    
    QRegExp remail("<?([^@\\s<]+@[^>\\s]+)>?"); // FIXME: user "proper" regexp,
       // search kmail source for it
    
    int pos = remail.search(str);
    if (pos != -1)
    {
        QString all = remail.cap(0);
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

    QRegExp rename("^\\(([^\\)]*)\\)");
    
    pos = rename.search(name);
    
    if (pos != -1)
    {
        name = rename.cap(1);
    }
    
    name = name.isEmpty() ? QString() : name;
    email = email.isEmpty() ? QString() : email;
}

QString parseItemAuthor(const QDomElement& element, Format format, Version version)
{
    QString name;
    QString email;

    QDomElement dcCreator = element.namedItem("dc:creator").toElement();
    
    if (!dcCreator.isNull())
         authorFromString(dcCreator.text(), name, email);
    else if (format == AtomFeed)
    {
        QDomElement atomAuthor = element.namedItem("author").toElement();
        if (atomAuthor.isNull())
            atomAuthor = element.namedItem("atom:author").toElement();
        if (!atomAuthor.isNull())
        {
            QDomElement atomName = atomAuthor.namedItem("name").toElement();
            if (atomName.isNull())
                atomName = atomAuthor.namedItem("atom:name").toElement();
            name = atomName.text().stripWhiteSpace();
            
            QDomElement atomEmail = atomAuthor.namedItem("email").toElement();
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
        return QString("<a href=\"mailto:%1\">%2</a>").arg(email).arg(name);
    else
        return name;
}

} // namespace RSS

// vim:noet:ts=4
