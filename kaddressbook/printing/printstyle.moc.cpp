/****************************************************************************
** KABPrinting::KABPrintStyle meta object code from reading C++ file 'printstyle.h'
**
** Created: Mon May 13 14:10:45 2002
**      by: The Qt MOC ($Id$)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "printstyle.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 19)
#error "This file was generated using the moc from 3.0.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *KABPrinting::KABPrintStyle::className() const
{
    return "KABPrinting::KABPrintStyle";
}

QMetaObject *KABPrinting::KABPrintStyle::metaObj = 0;
static QMetaObjectCleanUp cleanUp_KABPrinting__KABPrintStyle;

#ifndef QT_NO_TRANSLATION
QString KABPrinting::KABPrintStyle::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "KABPrinting::KABPrintStyle", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString KABPrinting::KABPrintStyle::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "KABPrinting::KABPrintStyle", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* KABPrinting::KABPrintStyle::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QObject::staticMetaObject();
    metaObj = QMetaObject::new_metaobject(
	"KABPrinting::KABPrintStyle", parentObject,
	0, 0,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_KABPrinting__KABPrintStyle.setMetaObject( metaObj );
    return metaObj;
}

void* KABPrinting::KABPrintStyle::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "KABPrinting::KABPrintStyle" ) ) return (KABPrinting::KABPrintStyle*)this;
    return QObject::qt_cast( clname );
}

bool KABPrinting::KABPrintStyle::qt_invoke( int _id, QUObject* _o )
{
    return QObject::qt_invoke(_id,_o);
}

bool KABPrinting::KABPrintStyle::qt_emit( int _id, QUObject* _o )
{
    return QObject::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool KABPrinting::KABPrintStyle::qt_property( int _id, int _f, QVariant* _v)
{
    return QObject::qt_property( _id, _f, _v);
}
#endif // QT_NO_PROPERTIES
