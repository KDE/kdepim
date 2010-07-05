
#include "../contactfields.h"

#include <qtest_kde.h>

#include <QtCore/QObject>

class ContactFieldsTest : public QObject
{
  Q_OBJECT

  private Q_SLOTS:
    void testFieldCount();
    void testSetGet();

  private:
    KABC::Addressee mContact;
};

QTEST_KDEMAIN( ContactFieldsTest, NoGUI )

static const QString s_formattedName( "User, Joe" );
static const QString s_prefix( "Mr." );
static const QString s_givenName( "Joe" );
static const QString s_additionalName( "Doe" );
static const QString s_familyName( "User" );
static const QString s_suffix( "Jr." );
static const QString s_nickName( "joe" );
static const QString s_birthday( "1966-12-03" );
static const QString s_anniversary( "1980-10-02" );
static const QString s_homeAddressStreet( "My Home Street" );
static const QString s_homeAddressPostOfficeBox( "My Home POB" );
static const QString s_homeAddressLocality( "My Home Locality" );
static const QString s_homeAddressRegion( "My Home Address" );
static const QString s_homeAddressPostalCode( "My Home Postal Code" );
static const QString s_homeAddressCountry( "My Home Country" );
static const QString s_homeAddressLabel( "My Home Label" );
static const QString s_businessAddressStreet( "My Business Street" );
static const QString s_businessAddressPostOfficeBox( "My Business POB" );
static const QString s_businessAddressLocality( "My Business Locality" );
static const QString s_businessAddressRegion( "My Business Region" );
static const QString s_businessAddressPostalCode( "My Business Postal Code" );
static const QString s_businessAddressCountry( "My Business Country" );
static const QString s_businessAddressLabel( "My Business Label" );
static const QString s_homePhone( "000111222" );
static const QString s_businessPhone( "333444555" );
static const QString s_mobilePhone( "666777888" );
static const QString s_homeFax( "999000111" );
static const QString s_businessFax( "222333444" );
static const QString s_carPhone( "555666777" );
static const QString s_isdn( "888999000" );
static const QString s_pager( "111222333" );
static const QString s_preferredEmail( "me@somewhere.org" );
static const QString s_email2( "you@somewhere.org" );
static const QString s_email3( "she@somewhere.org" );
static const QString s_email4( "it@somewhere.org" );
static const QString s_mailer( "kmail2" );
static const QString s_title( "Chief" );
static const QString s_role( "Developer" );
static const QString s_organization( "KDE" );
static const QString s_note( "That's a small note" );
static const QString s_homepage( "http://www.kde.de" );
static const QString s_blogFeed( "http://planetkde.org" );
static const QString s_profession( "Developer" );
static const QString s_office( "Room 2443" );
static const QString s_manager( "Hans" );
static const QString s_assistant( "Hins" );
static const QString s_spouse( "My Darling" );

void ContactFieldsTest::testFieldCount()
{
  QCOMPARE( ContactFields::allFields().count(), 48 );
}

void ContactFieldsTest::testSetGet()
{
  KABC::Addressee contact;

  ContactFields::setValue( ContactFields::FormattedName, s_formattedName, contact );
  ContactFields::setValue( ContactFields::Prefix, s_prefix, contact );
  ContactFields::setValue( ContactFields::GivenName, s_givenName, contact );
  ContactFields::setValue( ContactFields::AdditionalName, s_additionalName, contact );
  ContactFields::setValue( ContactFields::FamilyName, s_familyName, contact );
  ContactFields::setValue( ContactFields::Suffix, s_suffix, contact );
  ContactFields::setValue( ContactFields::NickName, s_nickName, contact );
  ContactFields::setValue( ContactFields::Birthday, s_birthday, contact );
  ContactFields::setValue( ContactFields::Anniversary, s_anniversary, contact );
  ContactFields::setValue( ContactFields::HomeAddressStreet, s_homeAddressStreet, contact );
  ContactFields::setValue( ContactFields::HomeAddressPostOfficeBox, s_homeAddressPostOfficeBox, contact );
  ContactFields::setValue( ContactFields::HomeAddressLocality, s_homeAddressLocality, contact );
  ContactFields::setValue( ContactFields::HomeAddressRegion, s_homeAddressRegion, contact );
  ContactFields::setValue( ContactFields::HomeAddressPostalCode, s_homeAddressPostalCode, contact );
  ContactFields::setValue( ContactFields::HomeAddressCountry, s_homeAddressCountry, contact );
  ContactFields::setValue( ContactFields::HomeAddressLabel, s_homeAddressLabel, contact );
  ContactFields::setValue( ContactFields::BusinessAddressStreet, s_businessAddressStreet, contact );
  ContactFields::setValue( ContactFields::BusinessAddressPostOfficeBox, s_businessAddressPostOfficeBox, contact );
  ContactFields::setValue( ContactFields::BusinessAddressLocality, s_businessAddressLocality, contact );
  ContactFields::setValue( ContactFields::BusinessAddressRegion, s_businessAddressRegion, contact );
  ContactFields::setValue( ContactFields::BusinessAddressPostalCode, s_businessAddressPostalCode, contact );
  ContactFields::setValue( ContactFields::BusinessAddressCountry, s_businessAddressCountry, contact );
  ContactFields::setValue( ContactFields::BusinessAddressLabel, s_businessAddressLabel, contact );
  ContactFields::setValue( ContactFields::HomePhone, s_homePhone, contact );
  ContactFields::setValue( ContactFields::BusinessPhone, s_businessPhone, contact );
  ContactFields::setValue( ContactFields::MobilePhone, s_mobilePhone, contact );
  ContactFields::setValue( ContactFields::HomeFax, s_homeFax, contact );
  ContactFields::setValue( ContactFields::BusinessFax, s_businessFax, contact );
  ContactFields::setValue( ContactFields::CarPhone, s_carPhone, contact );
  ContactFields::setValue( ContactFields::Isdn, s_isdn, contact );
  ContactFields::setValue( ContactFields::Pager, s_pager, contact );
  ContactFields::setValue( ContactFields::PreferredEmail, s_preferredEmail, contact );
  ContactFields::setValue( ContactFields::Email2, s_email2, contact );
  ContactFields::setValue( ContactFields::Email3, s_email3, contact );
  ContactFields::setValue( ContactFields::Email4, s_email4, contact );
  ContactFields::setValue( ContactFields::Mailer, s_mailer, contact );
  ContactFields::setValue( ContactFields::Title, s_title, contact );
  ContactFields::setValue( ContactFields::Role, s_role, contact );
  ContactFields::setValue( ContactFields::Organization, s_organization, contact );
  ContactFields::setValue( ContactFields::Note, s_note, contact );
  ContactFields::setValue( ContactFields::Homepage, s_homepage, contact );
  ContactFields::setValue( ContactFields::BlogFeed, s_blogFeed, contact );
  ContactFields::setValue( ContactFields::Profession, s_profession, contact );
  ContactFields::setValue( ContactFields::Office, s_office, contact );
  ContactFields::setValue( ContactFields::Manager, s_manager, contact );
  ContactFields::setValue( ContactFields::Assistant, s_assistant, contact );
  ContactFields::setValue( ContactFields::Anniversary, s_anniversary, contact );
  ContactFields::setValue( ContactFields::Spouse, s_spouse, contact );

  const KABC::Addressee contactCopy = contact;

  QCOMPARE( ContactFields::value( ContactFields::FormattedName, contactCopy ), s_formattedName );
  QCOMPARE( ContactFields::value( ContactFields::Prefix, contactCopy ), s_prefix );
  QCOMPARE( ContactFields::value( ContactFields::GivenName, contactCopy ), s_givenName );
  QCOMPARE( ContactFields::value( ContactFields::AdditionalName, contactCopy ), s_additionalName );
  QCOMPARE( ContactFields::value( ContactFields::FamilyName, contactCopy ), s_familyName );
  QCOMPARE( ContactFields::value( ContactFields::Suffix, contactCopy ), s_suffix );
  QCOMPARE( ContactFields::value( ContactFields::NickName, contactCopy ), s_nickName );
  QCOMPARE( ContactFields::value( ContactFields::Birthday, contactCopy ), s_birthday );
  QCOMPARE( ContactFields::value( ContactFields::Anniversary, contactCopy ), s_anniversary );
  QCOMPARE( ContactFields::value( ContactFields::HomeAddressStreet, contactCopy ), s_homeAddressStreet );
  QCOMPARE( ContactFields::value( ContactFields::HomeAddressPostOfficeBox, contactCopy ), s_homeAddressPostOfficeBox );
  QCOMPARE( ContactFields::value( ContactFields::HomeAddressLocality, contactCopy ), s_homeAddressLocality );
  QCOMPARE( ContactFields::value( ContactFields::HomeAddressRegion, contactCopy ), s_homeAddressRegion );
  QCOMPARE( ContactFields::value( ContactFields::HomeAddressPostalCode, contactCopy ), s_homeAddressPostalCode );
  QCOMPARE( ContactFields::value( ContactFields::HomeAddressCountry, contactCopy ), s_homeAddressCountry );
  QCOMPARE( ContactFields::value( ContactFields::HomeAddressLabel, contactCopy ), s_homeAddressLabel );
  QCOMPARE( ContactFields::value( ContactFields::BusinessAddressStreet, contactCopy ), s_businessAddressStreet );
  QCOMPARE( ContactFields::value( ContactFields::BusinessAddressPostOfficeBox, contactCopy ), s_businessAddressPostOfficeBox );
  QCOMPARE( ContactFields::value( ContactFields::BusinessAddressLocality, contactCopy ), s_businessAddressLocality );
  QCOMPARE( ContactFields::value( ContactFields::BusinessAddressRegion, contactCopy ), s_businessAddressRegion );
  QCOMPARE( ContactFields::value( ContactFields::BusinessAddressPostalCode, contactCopy ), s_businessAddressPostalCode );
  QCOMPARE( ContactFields::value( ContactFields::BusinessAddressCountry, contactCopy ), s_businessAddressCountry );
  QCOMPARE( ContactFields::value( ContactFields::BusinessAddressLabel, contactCopy ), s_businessAddressLabel );
  QCOMPARE( ContactFields::value( ContactFields::HomePhone, contactCopy ), s_homePhone );
  QCOMPARE( ContactFields::value( ContactFields::BusinessPhone, contactCopy ), s_businessPhone );
  QCOMPARE( ContactFields::value( ContactFields::MobilePhone, contactCopy ), s_mobilePhone );
  QCOMPARE( ContactFields::value( ContactFields::HomeFax, contactCopy ), s_homeFax );
  QCOMPARE( ContactFields::value( ContactFields::BusinessFax, contactCopy ), s_businessFax );
  QCOMPARE( ContactFields::value( ContactFields::CarPhone, contactCopy ), s_carPhone );
  QCOMPARE( ContactFields::value( ContactFields::Isdn, contactCopy ), s_isdn );
  QCOMPARE( ContactFields::value( ContactFields::Pager, contactCopy ), s_pager );
  QCOMPARE( ContactFields::value( ContactFields::PreferredEmail, contactCopy ), s_preferredEmail );
  QCOMPARE( ContactFields::value( ContactFields::Email2, contactCopy ), s_email2 );
  QCOMPARE( ContactFields::value( ContactFields::Email3, contactCopy ), s_email3 );
  QCOMPARE( ContactFields::value( ContactFields::Email4, contactCopy ), s_email4 );
  QCOMPARE( ContactFields::value( ContactFields::Mailer, contactCopy ), s_mailer );
  QCOMPARE( ContactFields::value( ContactFields::Title, contactCopy ), s_title );
  QCOMPARE( ContactFields::value( ContactFields::Role, contactCopy ), s_role );
  QCOMPARE( ContactFields::value( ContactFields::Organization, contactCopy ), s_organization );
  QCOMPARE( ContactFields::value( ContactFields::Note, contactCopy ), s_note );
  QCOMPARE( ContactFields::value( ContactFields::Homepage, contactCopy ), s_homepage );
  QCOMPARE( ContactFields::value( ContactFields::BlogFeed, contactCopy ), s_blogFeed );
  QCOMPARE( ContactFields::value( ContactFields::Profession, contactCopy ), s_profession );
  QCOMPARE( ContactFields::value( ContactFields::Office, contactCopy ), s_office );
  QCOMPARE( ContactFields::value( ContactFields::Manager, contactCopy ), s_manager );
  QCOMPARE( ContactFields::value( ContactFields::Assistant, contactCopy ), s_assistant );
  QCOMPARE( ContactFields::value( ContactFields::Anniversary, contactCopy ), s_anniversary );
  QCOMPARE( ContactFields::value( ContactFields::Spouse, contactCopy ), s_spouse );
}

#include "contactfieldstest.moc"
