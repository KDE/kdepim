#ifndef KTNEFDEFS_H
#define KTNEFDEFS_H

#define TNEF_SIGNATURE   0x223e9f78
#define LVL_MESSAGE      0x01
#define LVL_ATTACHMENT   0x02

#define atpSTRING   0x0001
#define atpTEXT     0x0002
#define atpDATE     0x0003
#define atpSHORT    0x0004
#define atpLONG     0x0005
#define atpBYTE     0x0006
#define atpWORD     0x0007
#define atpDWORD    0x0008

#define attDATESTART      0x0006
#define attDATEEND        0x0007
#define attAIDOWNER       0x0008
#define attREQUESTRES     0x0009
#define attFROM           0x8000
#define attSUBJECT        0x8004
#define attDATESENT       0x8005
#define attDATERECD       0x8006
#define attMSGSTATUS      0x8007
#define attMSGCLASS       0x8008
#define attMSGID          0x8009
#define attBODY           0x800c
#define attMSGPRIORITY    0x800d
#define attATTACHDATA	  0x800f	/* Attachment Data */
#define attATTACHTITLE	  0x8010	/* Attachment File Name */
#define attATTACHMETAFILE 0x8011
#define attATTACHMODDATE  0x8013
#define attDATEMODIFIED   0x8020
#define attATTACHRENDDATA 0x9002
#define attMAPIPROPS      0x9003
#define attRECIPTABLE     0x9004
#define attATTACHMENT	  0x9005	/* Attachment properties (?) */
#define attTNEFVERSION    0x9006
#define attOEMCODEPAGE    0x9007

/* These are found in TNEF documentation, but have so far not been implemented
#define attATTACHCREATEDATE        0x0000
#define attATTACHTRANSPORTFILENAME 0x0000
#define attCONVERSATIONID          0x0000
#define attORIGINALMSGCLASS        0x0000
#define attOWNER                   0x0000
#define attPARENTID                0x0000
#define attNULL                    0x0000
#define attDELEGATE                0x0000
#define attSENTFOR                 0x0000
*/

#define fmsModified   0x01
#define fmsLocal      0x02
#define fmsSubmitted  0x04
#define fmsRead       0x20
#define fmsHasAttach  0x80
#define MSGFLAG_READ       0x00000001
#define MSGFLAG_UNMODIFIED 0x00000002
#define MSGFLAG_SUBMIT     0x00000004
#define MSGFLAG_UNSENT     0x00000008
#define MSGFLAG_HASATTACH  0x00000010

// supported MAPI types
#define	MAPI_TYPE_NONE		0x0000
#define	MAPI_TYPE_UINT16	0x0002
#define	MAPI_TYPE_ULONG		0x0003
#define	MAPI_TYPE_FLOAT		0x0004
#define	MAPI_TYPE_DOUBLE	0x0005
#define MAPI_TYPE_BOOLEAN   0x000b
#define MAPI_TYPE_OBJECT    0x000d
#define	MAPI_TYPE_TIME		0x0040
#define	MAPI_TYPE_STRING8	0x001e
#define	MAPI_TYPE_USTRING	0x001f
#define	MAPI_TYPE_BINARY	0x0102
#define MAPI_TYPE_VECTOR    0x1000

// supported MAPI tags
#define	MAPI_TAG_INDEX		    0x0e21
#define	MAPI_TAG_SIZE		    0x0e20
#define	MAPI_TAG_FILENAME	    0x3707
#define	MAPI_TAG_DISPLAYNAME    0x3001
#define MAPI_TAG_DATA           0x3701
#define MAPI_TAG_ATTACHENCODING 0x3702
#define	MAPI_TAG_EXTENSION	    0x3703
#define MAPI_TAG_ATTACHMETHOD   0x3705
#define MAPI_TAG_RENDERINGPOS   0x370b
#define	MAPI_TAG_MIMETAG	    0x370e
#define MAPI_TAG_ATTACHFLAGS    0x3714

#define MAPI_IID_IMessage 0x00020307


// unsupported Outlook Contact Properties
// note: properties beginning with MAPI_TAG_CONTACT
//       are 'named' properties
#define MAPI_TAG_PR_MESSAGE_DELIVERY_TIME    0x0E060040  // CreationTime
#define MAPI_TAG_PR_MESSAGE_SIZE             0x0E080003  // Size
#define MAPI_TAG_PR_SENSITIVITY              0x00360003  // Sensitivity
#define MAPI_TAG_PR_MESSAGE_CLASS            0x001A001F  // MessageClass
#define MAPI_TAG_PR_IMPORTANCE               0x00170003  // Importance
#define MAPI_TAG_PR_ENTRYID                  0x0FFF0102  // EntryID
#define MAPI_TAG_PR_BODY                     0x1000001F  // Body
#define MAPI_TAG_CONTACT_Categories          "Keywords"  // Categories
#define MAPI_TAG_PR_DISPLAY_NAME             0x3001001F  // FullName
#define MAPI_TAG_PR_TITLE                    0x3A17001F  // JobTitle
#define MAPI_TAG_PR_COMPANY_NAME             0x3A16001F  // CompanyName
#define MAPI_TAG_PR_DISPLAY_NAME_PREFIX      0x3A45001E  // Title
#define MAPI_TAG_PR_SURNAME                  0x3A11001E  // LastName
#define MAPI_TAG_PR_MIDDLE_NAME              0x3A44001F  // MiddleName
#define MAPI_TAG_PR_GIVEN_NAME               0x3A06001F  // FirstName
#define MAPI_TAG_PR_GENERATION               0x3A05001E  // Suffix
#define MAPI_TAG_PR_BUSINESS_HOME_PAGE       0x3A51001F  // BusinessHomePage
#define MAPI_TAG_PR_PERSONAL_HOME_PAGE       0x3A50001F  // PersonalHomePage
#define MAPI_TAG_PR_FTP_SITE                 0x3A4C001E  // FTPSite
#define MAPI_TAG_PR_INITIALS                 0x3A0A001E  // Initials

#define MAPI_TAG_CONTACT_FILEUNDER                 "0x8005"    // FileAs

#define MAPI_TAG_CONTACT_LASTNAMEANDFIRSTNAME      "0x8017"    // LastNameandFirstName
#define MAPI_TAG_CONTACT_COMPANYANDFULLNAME        "0x8018"    // CompanyAndFullName
#define MAPI_TAG_CONTACT_FULLNAMEANDCOMPANY        "0x8019"    // FullNameAndCompany

#define MAPI_TAG_CONTACT_HOMEADDRESS               "0x801A"    // HomeAddress
#define MAPI_TAG_CONTACT_BUSINESSADDRESS           "0x801B"    // BusinessAddress
#define MAPI_TAG_CONTACT_OTHERADDRESS              "0x801C"    // OtherAddress
#define MAPI_TAG_CONTACT_SELECTEDADDRESS           "0x8022"    // SelectedMailingAddress:
                                                               // 0 = None
                                                               // 1 = Home                                                    
                                                               // 2 = Business
                                                               // 3 = Other

#define MAPI_TAG_CONTACT_WEBPAGE                   "0x802B"    // WebPage
#define MAPI_TAG_CONTACT_YOMIFIRSTNAME             "0x802C"    // YomiFirstName
#define MAPI_TAG_CONTACT_YOMILASTNAME              "0x802D"    // YomiLastName
#define MAPI_TAG_CONTACT_YOMICOMPANYNAME           "0x802E"    // YomiCompanyName

#define MAPI_TAG_CONTACT_LASTFIRSTNOSPACE          "0x8030"    // LastFirstNoSpace
#define MAPI_TAG_CONTACT_LASTFIRSTSPACEONLY        "0x8031"    // LastFirstSpaceOnly
#define MAPI_TAG_CONTACT_COMPANYLASTFIRSTNOSPACE   "0x8032"    // CompanyLastFirstNoSpace
#define MAPI_TAG_CONTACT_COMPANYLASTFIRSTSpaceOnly "0x8033"    // CompanyLastFirstSpaceOnly
#define MAPI_TAG_CONTACT_LASTFIRSTNOSPACECOMPANY   "0x8034"    // LastFirstNoSpaceCompany
#define MAPI_TAG_CONTACT_LASTFIRSTSPACEONLYCOMPANY "0x8035"    // LastFirstSpaceOnlyCompany
#define MAPI_TAG_CONTACT_LASTFIRSTANDSuffix        "0x8036"    // LastFirstAndSuffix

#define MAPI_TAG_CONTACT_BUSINESSADDRESSSTREET     "0x8045"    // BusinessAddressStreet
#define MAPI_TAG_CONTACT_BUSINESSADDRESSCITY       "0x8046"    // BusinessAddressCity
#define MAPI_TAG_CONTACT_BUSINESSADDRESSSTATE      "0x8047"    // BusinessAddressState
#define MAPI_TAG_CONTACT_BUSINESSADDRESSPOSTALCODE "0x8048"    // BusinessAddressPostalCode
#define MAPI_TAG_CONTACT_BUSINESSADDRESSCOUNTRY    "0x8049"    // BusinessAddressCountry
#define MAPI_TAG_CONTACT_BUSINESSADDRESSPOBOX      "0x804A"    // BusinessAddressPostOfficeBox

#define MAPI_TAG_CONTACT_USERFIELD1                "0x804F"    // UserField1
#define MAPI_TAG_CONTACT_USERFIELD2                "0x8050"    // UserField2
#define MAPI_TAG_CONTACT_USERFIELD3                "0x8051"    // UserField3
#define MAPI_TAG_CONTACT_USERFIELD4                "0x8052"    // UserField4

#define MAPI_TAG_CONTACT_IMADDRESS                 "0x8062"    // InternetMailAddress (only >= Outlook 2002)

#define MAPI_TAG_CONTACT_EMAIL1ADDRTYPE            "0x8082"    // EMail1AddressType
#define MAPI_TAG_CONTACT_EMAIL1EMAILADDRESS        "0x8083"    // EMail1Address
#define MAPI_TAG_CONTACT_EMAIL1ORIGINALDISPLAYNAME "0x8084"    // EMail1DisplayName
#define MAPI_TAG_CONTACT_EMAIL1ORIGINALENTRYID     "0x8085"    // EMail1EntryID

#define MAPI_TAG_CONTACT_EMAIL2ADDRTYPE            "0x8092"    // EMail2AddressType
#define MAPI_TAG_CONTACT_EMAIL2EMAILADDRESS        "0x8093"    // EMail2Address
#define MAPI_TAG_CONTACT_EMAIL2ORIGINALDISPLAYNAME "0x8094"    // EMail2DisplayName
#define MAPI_TAG_CONTACT_EMAIL2ORIGINALENTRYID     "0x8095"    // EMail2EntryID

#define MAPI_TAG_CONTACT_EMAIL3ADDRTYPE            "0x80A2"    // EMail3AddressType
#define MAPI_TAG_CONTACT_EMAIL3EMAILADDRESS        "0x80A3"    // EMail3Address
#define MAPI_TAG_CONTACT_EMAIL3ORIGINALDISPLAYNAME "0x80A4"    // EMail3DisplayName
#define MAPI_TAG_CONTACT_EMAIL3ORIGINALENTRYID     "0x80A5"    // EMail3EntryID

#define MAPI_TAG_CONTACT_INTERNETFREEBUSYADDRESS   "0x80D8"    // InternetFreeBusyAddress

#define MAPI_TAG_CONTACT_BILLINGINFORMATION        "0x8535"    // BillingInformation
#define MAPI_TAG_CONTACT_REMINDERTIME              "0x8502"    // N/A
#define MAPI_TAG_CONTACT_MILEAGE                   "0x8534"    // Mileage

#define MAPI_TAG_PR_ASSISTANT_TELEPHONE_NUMBER     0x3A2E001F  // AssistantTelephoneNumber
#define MAPI_TAG_PR_BUSINESS_TELEPHONE_NUMBER      0x3A08001F  // BusinessTelephoneNumber
#define MAPI_TAG_PR_BUSINESS2_TELEPHONE_NUMBER     0x3A1B001F  // Business2TelephoneNumber
#define MAPI_TAG_PR_BUSINESS_FAX_NUMBER            0x3A24001F  // BusinessFaxNumber
#define MAPI_TAG_PR_CALLBACK_TELEPHONE_NUMBER      0x3A02001F  // CallbackTelephoneNumber
#define MAPI_TAG_PR_CAR_TELEPHONE_NUMBER           0x3A1E001F  // CarTelephoneNumber

#define MAPI_TAG_PR_COMPANY_MAIN_PHONE_NUMBER      0x3A57001F  // CompanyMainTelephoneNumber

#define MAPI_TAG_PR_HOME_TELEPHONE_NUMBER          0x3A09001F  // HomeTelephoneNumber

#define MAPI_TAG_PR_HOME2_TELEPHONE_NUMBER         0x3A2F001F  // Home2TelephoneNumber
#define MAPI_TAG_PR_HOME_FAX_NUMBER                0x3A25001F  // HomeFaxNumber
#define MAPI_TAG_PR_ISDN_NUMBER                    0x3A2D001F  // ISDNNumber

#define MAPI_TAG_PR_MOBILE_TELEPHONE_NUMBER        0x3A1C001F  // MobileTelephoneNumber

#define MAPI_TAG_PR_OTHER_TELEPHONE_NUMBER         0x3A1F001F  // OtherTelephoneNumber
#define MAPI_TAG_PR_PRIMARY_FAX_NUMBER             0x3A23001F  // OtherFaxNumber
#define MAPI_TAG_PR_PAGER_TELEPHONE_NUMBER         0x3A21001F  // PagerNumber
#define MAPI_TAG_PR_PRIMARY_TELEPHONE_NUMBER       0x3A1A001F  // PrimaryTelephoneNumber

#define MAPI_TAG_PR_RADIO_TELEPHONE_NUMBER         0x3A1D001F  // RadioTelephoneNumber

#define MAPI_TAG_PR_TELEX_NUMBER                   0x3A2C001F  // TelexNumber
#define MAPI_TAG_PR_TTYTDD_PHONE_NUMBER            0x3A4B001F  // TTYTDDTelephoneNumber
#define MAPI_TAG_PR_POSTAL_ADDRESS                 0x3A15001F  // MailingAddress

#define MAPI_TAG_PR_BUSINESS_ADDRESS_COUNTRY       0x3A26001E  // MailingAddressCountry
#define MAPI_TAG_PR_LOCALITY                       0x3A27001F  // MailingAddressCity
#define MAPI_TAG_PR_STATE_OR_PROVINCE              0x3A28001E  // MailingAddressState
#define MAPI_TAG_PR_STREET_ADDRESS                 0x3A29001F  // MailingAddressStreet
#define MAPI_TAG_PR_POSTAL_CODE                    0x3A2A001E  // MailingAddressPostalCode
#define MAPI_TAG_PR_PO_BOX                         0x3A2B001E  // MailingAddressPostOfficeBox

#define MAPI_TAG_PR_HOME_ADDRESS_CITY              0x3A59001E  // HomeAddressCity
#define MAPI_TAG_PR_HOME_ADDRESS_STREET            0x3A5D001E  // HomeAddressStreet
#define MAPI_TAG_PR_HOME_ADDRESS_STATE_OR_PROVINCE 0x3A5C001E  // HomeAddressState
#define MAPI_TAG_PR_HOME_ADDRESS_COUNTRY           0x3A5A001E  // HomeAddressCountry
#define MAPI_TAG_PR_HOME_ADDRESS_PO_BOX            0x3A5E001E  // HomeAddressPostOfficeBox
#define MAPI_TAG_PR_HOME_ADDRESS_POSTAL_CODE       0x3A5B001E  // HomeAddressPostalCode

#define MAPI_TAG_PR_OTHER_ADDRESS_CITY             0x3A5F001E  // OtherAddressCity
#define MAPI_TAG_PR_OTHER_ADDRESS_STREET           0x3A63001E  // OtherAddressStreet
#define MAPI_TAG_PR_OTHER_ADDRESS_STATE_OR_PROVINCE 0x3A62001E // OtherAddressState
#define MAPI_TAG_PR_OTHER_ADDRESS_COUNTRY           0x3A60001E // OtherAddressCountry
#define MAPI_TAG_PR_OTHER_ADDRESS_POSTAL_CODE       0x3A61001E // OtherAddressPostalCode
#define MAPI_TAG_PR_OTHER_ADDRESS_PO_BOX            0x3A64001E // OtherAddressPostOfficeBox

#define MAPI_TAG_PR_DEPARTMENT_NAME                 0x3A18001F // Department
#define MAPI_TAG_PR_MANAGER_NAME                    0x3A4E001F // ManagerName
#define MAPI_TAG_PR_OFFICE_LOCATION                 0x3A19001F // Location
#define MAPI_TAG_PR_ASSISTANT                       0x3A30001F // AssistantName
#define MAPI_TAG_PR_PROFESSION                      0x3A46001F // Profession
#define MAPI_TAG_PR_NICKNAME                        0x3A4F001F // NickName
#define MAPI_TAG_PR_BIRTHDAY                        0x3A420040 // Birthday
#define MAPI_TAG_PR_SPOUSE_NAME                     0x3A48001F // SpouseName
#define MAPI_TAG_PR_WEDDING_ANNIVERSARY             0x3A410040 // Anniversary
#define MAPI_TAG_PR_ACCOUNT                         0x3A00001E // Account
#define MAPI_TAG_PR_COMPUTER_NETWORK_NAME           0x3A49001E // ComputerNetworkName
#define MAPI_TAG_PR_CHILDRENS_NAMES                 0x3A58101E // Children
#define MAPI_TAG_PR_CUSTOMER_ID                     0x3A4A001E // CustomerID
#define MAPI_TAG_PR_GENDER                          0x3A4D0002 // Gender:
                                                               // 0 = Unspecified
                                                               // 1 = Female
                                                               // 2 = Male
#define MAPI_TAG_PR_GOVERNMENT_ID_NUMBER            0x3A07001E // GovernmentIDNumber
#define MAPI_TAG_PR_HOBBIES                         0x3A43001E // Hobby
#define MAPI_TAG_PR_LANGUAGE                        0x3A0C001E // Language
#define MAPI_TAG_PR_LOCATION                        0x3A0D001E // OfficeLocation
#define MAPI_TAG_PR_ORGANIZATIONAL_ID_NUMBER        0x3A10001E // OrganizationalIDNumber
#define MAPI_TAG_PR_REFERRED_BY_NAME                0X3A47001E // ReferredBy

#endif /* KTNEFDEFS_H */
