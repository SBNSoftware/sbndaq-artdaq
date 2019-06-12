#ifndef NEVISTPC_DAQEXCEPTIONS_H
#define NEVISTPC_DAQEXCEPTIONS_H  1

#include <exception>
#include <string>

namespace nevistpc {
 
class daqexception : public std::exception
{
public:
    daqexception(std::string const& message, std::string const& name="daqexception");
    virtual const char *    what () const throw ();
    virtual ~daqexception() noexcept;

private:
    std::string _name;
    std::string _message;
};

class InvalidConfigurationException : public daqexception
{
public:
    InvalidConfigurationException(std::string const& message, std::string const& name="InvalidConfiguration Exception");
    virtual ~InvalidConfigurationException()  noexcept;
};

class InvalidFileFormatException : public daqexception
{
public:
    InvalidFileFormatException(std::string const& message, std::string const& name="InvalidFileFormatException");
    virtual ~InvalidFileFormatException()  noexcept;
};

class TerminateProcessException : public daqexception
{
public:
    TerminateProcessException(std::string const& message, std::string const& name="TerminateProcessException");
    virtual ~TerminateProcessException()  noexcept;
};

class RuntimeErrorException : public daqexception
{
public:
    RuntimeErrorException(std::string const& message, std::string const& name="RuntimeErrorException");
    virtual ~RuntimeErrorException()  noexcept;
};


class DataNotFound : public daqexception
{
public:
    DataNotFound(std::string const& message, std::string const& name="DataNotFound");
    virtual ~DataNotFound()  noexcept;
};

class NetworkIOException : public daqexception
{
public:
    NetworkIOException(std::string const& message, std::string const& name="NetworkIOException");
    virtual ~NetworkIOException()  noexcept;
};

class DataBufferIOException : public daqexception
{
public:
    DataBufferIOException(std::string const& message, std::string const& name="DataBufferIOException");
    virtual ~DataBufferIOException()  noexcept;
};


}  // end of namespace nevistpc

#endif //DAQEXCEPTIONS_H

// kate: indent-mode cstyle; replace-tabs on; 

