#ifndef __KLEOPATRA_UTILS_WSASTARTER_H__
#define __KLEOPATRA_UTILS_WSASTARTER_H__

namespace Kleo {

struct WSAStarter {
    const int startupError;

    WSAStarter();
    ~WSAStarter();
};

} // namespace Kleo

#endif // __KLEOPATRA_UTILS_WSASTARTER_H__
