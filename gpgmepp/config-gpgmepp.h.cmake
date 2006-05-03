/* Always set since we use gpgme-copy if gpgme isn't available */
#define HAVE_GPGME_0_4_BRANCH 1

/* Define to 1 if your gpgme supports gpgme_cancel() */
#cmakedefine HAVE_GPGME_CANCEL 1

/* Define to 1 if your gpgme has the GPGME_INCLUDE_CERTS_DEFAULT macro */
#cmakedefine HAVE_GPGME_INCLUDE_CERTS_DEFAULT 1

/* Define to 1 if your gpgme supports GPGME_KEYLIST_MODE_VALIDATE */
#cmakedefine HAVE_GPGME_KEYLIST_MODE_VALIDATE 1

/* Define to 1 if your gpgme's gpgme_key_t has the is_qualified flag */
#cmakedefine HAVE_GPGME_KEY_T_IS_QUALIFIED 1

/* Define to 1 if your gpgme's gpgme_key_t has the keylist_mode member */
#cmakedefine HAVE_GPGME_KEY_T_KEYLIST_MODE 1

/* Define to 1 if your gpgme's gpgme_subkey_t has the is_qualified flag */
#cmakedefine HAVE_GPGME_SUBKEY_T_IS_QUALIFIED 1

/* Define to 1 if your gpgme's gpgme_decrypt_result_t has the wrong_key_usage
   member */
#cmakedefine HAVE_GPGME_WRONG_KEY_USAGE 1

