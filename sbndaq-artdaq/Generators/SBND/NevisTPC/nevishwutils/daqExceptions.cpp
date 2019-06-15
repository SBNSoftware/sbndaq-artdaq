#include <sstream>
#include "daqExceptions.h"

namespace nevistpc {

daqexception::daqexception(std::string const& message, std::string const& name)
    :_name(name), _message(message)
{
    std::stringstream out;
    out << " " << _name << " Message: " << _message;
    _message = out.str();
}

const char *    daqexception::what () const throw ()
{
    return _message.c_str();
}

daqexception::~daqexception() noexcept
{
}

InvalidConfigurationException::InvalidConfigurationException(std::string const& message, std::string const& name)
    :daqexception(message,name)
{
}

InvalidConfigurationException::~InvalidConfigurationException() noexcept
{
}

InvalidFileFormatException::InvalidFileFormatException(std::string const& message, std::string const& name)
    :daqexception(message,name)
{
}

InvalidFileFormatException::~InvalidFileFormatException() noexcept
{
}

TerminateProcessException::TerminateProcessException(std::string const& message, std::string const& name)
    :daqexception(message,name)
{
}

TerminateProcessException::~TerminateProcessException() noexcept
{
}

DataNotFound::DataNotFound(std::string const& message, std::string const& name)
    :daqexception(message,name)
{
}

DataNotFound::~DataNotFound() noexcept
{
}

RuntimeErrorException::RuntimeErrorException(std::string const& message, std::string const& name)
    :daqexception(message,name)
{
}

RuntimeErrorException::~RuntimeErrorException() noexcept
{
}


NetworkIOException::NetworkIOException(std::string const& message, std::string const& name)
    :daqexception(message,name)
{
}

NetworkIOException::~NetworkIOException() noexcept
{
}

DataBufferIOException::DataBufferIOException(std::string const& message, std::string const& name)
    :daqexception(message,name)
{
}


DataBufferIOException::~DataBufferIOException() noexcept
{
}

  
}  // end of namespace nevistpc

// kate: indent-mode cstyle; replace-tabs on; 

