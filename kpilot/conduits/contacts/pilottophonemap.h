/**
 * Okay, this is so that we can map the Pilot phone types to Phone Number
 * types. Email addresses are NOT included in this map, and are handled
 * separately (not in PhoneNumber at all). The Pilot has 8 different kinds
 * of phone numbers (which may be *labeled* however you like). These
 * need to be mapped to the things that KABC::PhoneNumber handles.
 *
 * From KABC::PhoneNumber
 *		enum Types { Home = 1, Work = 2, Msg = 4, Pref = 8, Voice = 16, Fax = 32,
 *				Cell = 64, Video = 128, Bbs = 256, Modem = 512, Car = 1024,
 *				Isdn = 2048, Pcs = 4096, Pager = 8192 };
 *
 *
 * From PilotAddress:
 * enum EPhoneType {
 *		eWork=0, eHome, eFax, eOther, eEmail, eMain,
 *		ePager, eMobile
 *		};
 *
 * This array must have as many elements as PilotAddress::PhoneType
 * and its elements must be KABC::PhoneNumber::Types.
 */
static KABC::PhoneNumber::TypeFlag pilotToPhoneMap[8] = {
	KABC::PhoneNumber::Work,  // eWork
	KABC::PhoneNumber::Home,  // eHome,
	KABC::PhoneNumber::Fax,   // eFax,
	(KABC::PhoneNumber::TypeFlag)0, // eOther -> wasn't mapped properly,
	(KABC::PhoneNumber::TypeFlag)0, // eEmail -> shouldn't occur,
	KABC::PhoneNumber::Home,  // eMain
	KABC::PhoneNumber::Pager, // ePager,
	KABC::PhoneNumber::Cell   // eMobile
};
