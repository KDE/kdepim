#ifndef KGPGCONF_EXCEPTION_H
#define KGPGCONF_EXCEPTION_H

#include <QString>

class KGpgConfException {
public:
    explicit KGpgConfException( const QString& msg=QString() ) : m_message( msg ) {} 
        virtual ~KGpgConfException() throw();

        QString message() const { return m_message; }

    private:
        const QString m_message;
};

class GpgConfRunException : public KGpgConfException
{
public:
GpgConfRunException( int errorCode, const QString& msg ) : KGpgConfException( msg ), m_errorCode( errorCode ) {}
    ~GpgConfRunException() throw();

    int errorCode() const { return m_errorCode; }

private:
    int m_errorCode;
};

class MalformedGpgConfOutputException : public KGpgConfException
{
public:
    explicit MalformedGpgConfOutputException( const QString& msg ) : KGpgConfException( msg ) {}
    ~MalformedGpgConfOutputException() throw();

};

#endif /* KGPGCONF_EXCEPTION_H */
