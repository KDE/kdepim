# gpgme configure checks

set(CMAKE_REQUIRED_INCLUDES ${GPGME_INCLUDES})
set(CMAKE_REQUIRED_LIBRARIES ${GPGME_LIBRARIES})

# check if gpgme has GPGME_KEYLIST_MODE_VALIDATE
check_cxx_source_compiles ("
  #include <gpgme.h>
  int main() {
    gpgme_keylist_mode_t mode = GPGME_KEYLIST_MODE_VALIDATE;
  }
" HAVE_GPGME_KEYLIST_MODE_VALIDATE
)

# check if gpgme has gpgme_cancel
check_cxx_source_compiles ("
  #include <gpgme.h>
  int main() {
    gpgme_ctx_t ctx = 0;
    gpgme_error_t e = gpgme_cancel( ctx );
  }
" HAVE_GPGME_CANCEL 
)

# check if gpgme has gpgme_key_t->keylist_mode
check_cxx_source_compiles ("
  #include <gpgme.h>
  int main() {
    gpgme_key_t key = 0;
    key->keylist_mode = 0;
  }
" HAVE_GPGME_KEY_T_KEYLIST_MODE 
)

# check if gpgme has gpgme_decrypt_result_t->wrong_key_usage
check_cxx_source_compiles ("
  #include <gpgme.h>
  int main() {
    gpgme_decrypt_result_t res;
    unsigned int wku = res->wrong_key_usage;
  }
" HAVE_GPGME_WRONG_KEY_USAGE
)

# check if gpgme has GPGME_INCLUDE_CERTS_DEFAULT
check_cxx_source_compiles ("
  #include <gpgme.h>
  int main() {
    int i = GPGME_INCLUDE_CERTS_DEFAULT;
  }
" HAVE_GPGME_INCLUDE_CERTS_DEFAULT
)

# check if gpgme has gpgme_key_t->is_qualified
check_cxx_source_compiles ("
  #include <gpgme.h>
  int main() {
    gpgme_key_t key;
    unsigned int iq;
    iq = key->is_qualified;
  }
" HAVE_GPGME_KEY_T_IS_QUALIFIED 
)

# check if gpgme has gpgme_subkey_t->is_qualified
check_cxx_source_compiles ("
  #include <gpgme.h>
  int main() {
    gpgme_subkey_t subkey;
    unsigned int iq;
    iq = subkey->is_qualified;
  }
" HAVE_GPGME_SUBKEY_T_IS_QUALIFIED 
)

set(CMAKE_REQUIRED_INCLUDES)
set(CMAKE_REQUIRED_LIBRARIES)

