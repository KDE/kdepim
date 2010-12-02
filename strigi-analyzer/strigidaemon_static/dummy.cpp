#include <strigi/analyzerplugin.h>

IMPORT_PLUGIN(Strigi_Plugin_Ctg)
IMPORT_PLUGIN(Strigi_Plugin_Ics)
IMPORT_PLUGIN(Strigi_Plugin_Mail)
IMPORT_PLUGIN(Strigi_Plugin_Vcf)

#include <QtCore/QtPlugin>

Q_IMPORT_PLUGIN(akonadi_serializer_mail)
Q_IMPORT_PLUGIN(akonadi_serializer_addressee)
Q_IMPORT_PLUGIN(akonadi_serializer_contactgroup)
Q_IMPORT_PLUGIN(akonadi_serializer_kcalcore)