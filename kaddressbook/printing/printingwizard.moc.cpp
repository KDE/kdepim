/****************************************************************************
** KABPrinting::PrintingWizard meta object code from reading C++ file 'printingwizard.h'
**
** Created: Mon May 13 14:04:36 2002
**      by: The Qt MOC ($Id$)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "printingwizard.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 19)
#error "This file was generated using the moc from 3.0.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *KABPrinting::PrintingWizard::className() const
{
    return "KABPrinting::PrintingWizard";
}

QMetaObject *KABPrinting::PrintingWizard::metaObj = 0;
static QMetaObjectCleanUp cleanUp_KABPrinting__PrintingWizard;

#ifndef QT_NO_TRANSLATION
QString KABPrinting::PrintingWizard::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "KABPrinting::PrintingWizard", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString KABPrinting::PrintingWizard::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "KABPrinting::PrintingWizard", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* KABPrinting::PrintingWizard::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = PrintingWizardBase::staticMetaObject();
    metaObj = QMetaObject::new_metaobject(
	"KABPrinting::PrintingWizard", parentObject,
	0, 0,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_KABPrinting__PrintingWizard.setMetaObject( metaObj );
    return metaObj;
}

void* KABPrinting::PrintingWizard::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "KABPrinting::PrintingWizard" ) ) return (KABPrinting::PrintingWizard*)this;
    return PrintingWizardBase::qt_cast( clname );
}

bool KABPrinting::PrintingWizard::qt_invoke( int _id, QUObject* _o )
{
    return PrintingWizardBase::qt_invoke(_id,_o);
}

bool KABPrinting::PrintingWizard::qt_emit( int _id, QUObject* _o )
{
    return PrintingWizardBase::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool KABPrinting::PrintingWizard::qt_property( int _id, int _f, QVariant* _v)
{
    return PrintingWizardBase::qt_property( _id, _f, _v);
}
#endif // QT_NO_PROPERTIES
