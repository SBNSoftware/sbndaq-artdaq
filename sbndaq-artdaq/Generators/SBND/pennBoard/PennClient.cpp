/*
 * PennClient.cpp
 *
 *  Created on: Dec 15, 2014
 *      Author: tdealtry (based on tcn45 rce code)
 */

#include "PennClient.hh"

#include <iostream>
#include <sstream>
#include <boost/algorithm/string.hpp>
#include "libxml/parser.h"
#include "libxml/tree.h"

//#define __PTB_DEVEL_MODE__

sbnddaq::PennClient::PennClient(const std::string& host_name, const std::string& port_or_service, const unsigned int timeout_usecs) :
socket_(io_service_),
deadline_(io_service_),
timeout_usecs_(timeout_usecs),
exception_(false)
{
#ifdef __PTB_DEVEL_MODE__
  mf::LogDebug("PennClient") << "sbnddaq::PennClient constructor";
#endif
  // Initialise deadline timer to positive infinity so that no action will be taken until a
  // deadline is set
  deadline_.expires_at(boost::posix_time::pos_infin);

  // Start the persistent deadline actor that checks for timeouts
  this->check_deadline();

  // Attempt to establish the connection to the PENN, using a timeout if necessary
  try
  {

    // Resolve possible endpoints from specified host and port/service
    tcp::resolver resolver(io_service_);
    tcp::resolver::iterator endpoint_iter = resolver.resolve(tcp::resolver::query(host_name, port_or_service));
    tcp::resolver::iterator end;

    while ((endpoint_iter != end) && (socket_.is_open() == false))
    {
      tcp::endpoint endpoint = *endpoint_iter++;
      mf::LogInfo("PennClient") << "Connecting to PTB at " << endpoint;

      // If a client timeout is specified, set the deadline timer appropriately
      this->set_deadline();

      // Set error code to would_block as async operations guarantee not to set this value
      boost::system::error_code error = boost::asio::error::would_block;

      // Start the asynchronous connection attempt, which will call the appropriate handler
      // on completion
      socket_.async_connect(endpoint, boost::bind(&sbnddaq::PennClient::async_connect_handler, _1, &error));

      // Run the IO service and block until the connection handler completes
      do
      {
        io_service_.run_one();
      }
      while (error == boost::asio::error::would_block);

      // If the deadline timer has been called and cancelled the socket operation before the
      // connection has established, a timeout occurred
      if (error == boost::asio::error::operation_aborted)
      {
        socket_.close();
        mf::LogError("PennClient") << "Timeout establishing client connection to PTB at " << endpoint;
        // TODO replace with exception
      }
      // If another error occurred during connect - throw an exception
      else if (error)
      {
        socket_.close();

        // JCF, Jul-29-2015

        // Swallow exception thrown; does not necessarily prevent datataking
        try {
          mf::LogError("PennClient") << "Error establishing connection to PTB at " << endpoint << " : " << error.message();
        } catch (...) {} // Swallow
      }
    }
    if (socket_.is_open() == false)
    {
      mf::LogError("PennClient") << "Failed to open connection to PENN";
    } else {

      // Flush the socket of any stale aysnc update data from PENN
      size_t bytesFlushed = 0;
      std::string data;
      do {
        data = this->receive();
        bytesFlushed += data.length();
      } while (data.length() > 0);
      mf::LogInfo("PennClient") << "Flushed " << bytesFlushed << " bytes of stale data from client socket";

    }
  }
  catch (boost::system::system_error& e)
  {
    mf::LogError("PennClient") << "Exception caught opening connection to PENN: " << e.what();
  }

}

sbnddaq::PennClient::~PennClient()
{
  try {
    socket_.close();
  }
  catch (boost::system::system_error& e)
  {
    mf::LogError("PennClient") << "Exception caught closing PennClient connection:" << e.what();
  }
}

void sbnddaq::PennClient::send_command(std::string const & command)
{
  mf::LogInfo("PennClient") << "Sending command: " << command;

  // Build the XML fragment with command as empty closed tag
  std::ostringstream xml_frag;
  xml_frag << "<command>" << command << "</command>";

  // Send it
  this->send_xml(xml_frag.str());
}

void sbnddaq::PennClient::send_config(std::string const & config)
{
#ifdef __PTB_DEVEL_MODE__
  mf::LogDebug("PennClient") << "Sending config: " << config;
#endif

  this->send_xml(config);

  // If anything else than the status is expected, it is returned by ptb_answer

  //	std::ostringstream config_frag;
  //	config_frag << "<config>" << config << "</config>";
  //
  //	this->send_xml(config_frag.str());
}

void sbnddaq::PennClient::send_xml(std::string const & xml_frag)
{

  // Wrap the XML fragment for this request in the root system tags
  std::ostringstream xml_cmd;
  xml_cmd << xml_frag ;

  // Send the XML request to the PTB
  this->send(xml_cmd.str());

  // Get the response
  xmlDocPtr doc;
  std::string response = "";

  int max_response_timeout_us = 10000000;
  int max_retries = max_response_timeout_us / timeout_usecs_;
  int retries = 0;

  while ( retries++ < max_retries) {
    // NFB : It seems that the response arrived incomplete.
    response += this->receive();
    // mf::LogInfo("PennClient") << "Received answer : [" << response << "]";
    //	  doc = xmlReadMemory(response.c_str(), response.length()-1, "noname.xml", NULL, 0);
    doc = xmlReadMemory(response.c_str(), response.length(), "noname.xml", NULL, 0);
    if(doc != NULL) {
      break;
    }
  }//while(retries)

  // Traverse the DOM of the XML response and determine if any of the child elements are error.
  if (doc == NULL) {

    // JCF, Jul-29-2015

    // I'm swallowing the exception thrown by LogError here --
    // in my experience, this issue doesn't prevent datataking

    // NFB, Jan-16-2016

    // This should probably be an error that would lead the run to stop.
    // The PTB is supposed to always answer to a control communication command. If an answer is not returned
    // I would expect that something went wrong and the run is likely to fail.
    try {
      mf::LogWarning("PennClient") << "Failed to parse XML response: [" << response <<  "] (length " << response.length() << "). There is almost certainly a problem with this run.";
    } catch (...) {
      set_exception(true);
    } // Swallow
  }
  else {
    /*Get the root element node */
    xmlNode* root_element = xmlDocGetRootElement(doc);
    xmlNode *cur_node = NULL;
    for (cur_node = root_element->children; cur_node; cur_node = cur_node->next) {
      if (cur_node->type == XML_ELEMENT_NODE) {
        if (std::string((const char*)(cur_node->name)) == "success")
        {
          xmlChar* msgContent = xmlNodeGetContent(cur_node);
          if (std::string((const char*)(msgContent)) == "true") {
            mf::LogInfo("PennClient") << "Command executed successfully on the PTB";
          } else {
            try{
              mf::LogError("PennClient") << "Command failed execution on the PTB. Run is most likely going to fail.";
            } catch (...) {
              set_exception(true);
            } // swallow
          }
          xmlFree(msgContent);
        }
        else if (std::string((const char*)(cur_node->name)) == "error")
        {
          //xmlNode* error_node = cur_node->children;
          xmlChar* errorContent = xmlNodeGetContent(cur_node);
          if (errorContent) {
            mf::LogError("PennClient") << "Got error response from PTB: " << errorContent;
          } else {
            mf::LogError("PennClient") << "Got error response from PTB but cannot parse content";
          }
          xmlFree(errorContent);
        }
        else if (std::string((const char*)(cur_node->name)) == "warning")
        {
          xmlChar* warnContent = xmlNodeGetContent(cur_node);
          if (warnContent) {
            mf::LogWarning("PennClient") << "Got warning response from PTB: " << warnContent;
          } else {
            mf::LogWarning("PennClient") << "Got warning response from PTB but cannot parse content";
          }
          xmlFree(warnContent);
        }
        else if (std::string((const char*)(cur_node->name)) == "run_statistics") {
          xmlAttrPtr pAttr = cur_node->properties;
          std::ostringstream msg;
          msg << "PTB end of run statistics : " << std::endl;

          while(pAttr) {
            if (pAttr->type == XML_ATTRIBUTE_NODE) {
              std::string attr_name((const char*)pAttr->name);
              std::string attr_value((const char*)pAttr->children->content);

              msg << " [" << std::setw(25) << attr_name << " ] : " << std::setw(10) << attr_value << "\n" << std::endl;
            }
            pAttr = pAttr->next;
          }
          mf::LogInfo("PennClient") << msg.str() << "\n\n" << std::endl << std::endl;

        } else {
          mf::LogWarning("PennClient") << "Got an unrecognized answer from PTB with name " << (const char*)(cur_node->name);
          xmlChar* answerContent = xmlNodeGetContent(cur_node);
          if (answerContent) {
            mf::LogWarning("PennClient") << " and content : " << answerContent;
          } else {
            mf::LogWarning("PennClient") << " and content unknown content.";
          }
        }
      } else if (cur_node->type == XML_TEXT_NODE) {
        xmlChar *val = xmlNodeGetContent(cur_node);
        mf::LogWarning("PennClient") << "Received unrecognized response : " << val;
        xmlFree(val);

      } else {
        mf::LogWarning("PennClient") << "Received unrecognized XML element type from PTB : [" << cur_node->type << "]";
      }
    }
    xmlFreeDoc(doc);
  }//doc != NULL
}

// ------------ Private methods ---------------

void sbnddaq::PennClient::set_deadline(void)
{
  // Set the deadline for the asynchronous write operation if the timeout is set, otherwise
  // revert back to +ve infinity to stall the deadline timer actor
  if (timeout_usecs_ > 0)
  {
    deadline_.expires_from_now(boost::posix_time::microseconds(timeout_usecs_));
  }
  else
  {
    deadline_.expires_from_now(boost::posix_time::pos_infin);
  }
}
void sbnddaq::PennClient::check_deadline(void)
{

  // Handle deadline if expired
  if (deadline_.expires_at() <= boost::asio::deadline_timer::traits_type::now())
  {
    // Cancel pending socket operation
    socket_.cancel();

    // No longer an active deadline to set the expiry to positive inifinity
    deadline_.expires_at(boost::posix_time::pos_infin);
  }

  // Put the deadline actor back to sleep
  deadline_.async_wait(boost::bind(&sbnddaq::PennClient::check_deadline, this));
}

void sbnddaq::PennClient::async_connect_handler(const boost::system::error_code& ec, boost::system::error_code* output_ec)
{
  *output_ec = ec;
}

void sbnddaq::PennClient::async_completion_handler(
    const boost::system::error_code &error_code, std::size_t length,
    boost::system::error_code* output_error_code, std::size_t* output_length)
{
  *output_error_code = error_code;
  *output_length = length;
}

std::size_t sbnddaq::PennClient::send(std::string const & send_str)
{

  // Set the deadline to enable timeout on send if appropriate
  this->set_deadline();

  // Set up variables to receive the result of the async operation. The error code is set
  // to would_block as ASIO guarantees that its async calls never fail with this value - any
  // other value therefore indicates completion or failure
  std::size_t send_len = 0;
  boost::system::error_code error = boost::asio::error::would_block;

  // Start the async operation. The asyncCompletionHandler function is bound as a callback
  // to update the error and length variables
  boost::asio::async_write(socket_, boost::asio::buffer(send_str),
      boost::bind(&sbnddaq::PennClient::async_completion_handler, _1, _2, &error, &send_len));

  // Block until the async operation has completed
  do
  {
    io_service_.run_one();
  }
  while (error == boost::asio::error::would_block);

  // Handle any error conditions arising during the async operation
  if (error == boost::asio::error::eof)
  {
    // Connection closed by peer
    mf::LogError("PennClient") << "Connection closed by PENN";
  }
  else if (error == boost::asio::error::operation_aborted)
  {
    // Timeout signalled by deadline actor
    mf::LogError("PennClient") << "Timeout sending message to PENN";
  }
  else if (error)
  {
    mf::LogError("PennClient") << "Error sending message to PENN: " << error.message();
  }
  else if (send_len != send_str.size())
  {
    mf::LogError("PennClient") << "Size mismatch when sending transaction: wrote " << send_len << " expected " << send_str.size();
  }

  return send_len;
}

// std::string sbnddaq::PennClient::receive(void)
// {

// 	this->set_deadline();

// 	std::size_t receive_length = 0;
// 	boost::system::error_code error = boost::asio::error::would_block;

// 	boost::array<char, 128> raw_buffer;

// 	socket_.async_read_some(boost::asio::buffer(raw_buffer),
// 			boost::bind(&sbnddaq::PennClient::async_completion_handler, _1, _2, &error, &receive_length));

// 	do
// 	{
// 		io_service_.run_one();
// 	}
// 	while (error == boost::asio::error::would_block);

// 	return std::string(raw_buffer.data(), receive_length);
// }

std::string sbnddaq::PennClient::receive(void)
{
  this->set_deadline();
  std::size_t receive_length = 0;
  boost::system::error_code error = boost::asio::error::would_block;

  boost::asio::streambuf response;

  boost::asio::async_read_until(socket_, response, "\f",
      boost::bind(&sbnddaq::PennClient::async_completion_handler, _1, _2, &error, &receive_length));

  do
  {
    io_service_.run_one();
  }
  while (error == boost::asio::error::would_block);

  boost::asio::streambuf::const_buffers_type bufs = response.data();
  return std::string(boost::asio::buffers_begin(bufs), boost::asio::buffers_begin(bufs) + response.size());

}

void sbnddaq::PennClient::set_param_(std::string const & name, std::string const & encoded_value, std::string const & type)
{
  // Encode the parameter into a command with argument list
  std::ostringstream msg;
  msg << "SET"
      << " param=" << name
      << " value=" << encoded_value
      << " type="  << type << std::endl;

  // Send it
  this->send(msg.str());

  // Get the response
  std::string response = this->receive();

  // Parse the response to ensure the command was acknowledged
  if (!response_is_ack(response, "SET"))
  {
    mf::LogError("PennClient") << "SET command failed: " << response;
  }

}

bool sbnddaq::PennClient::response_is_ack(std::string const & response, std::string const & command)
{
  bool is_ack = true;
  std::vector<std::string> tokens;

  boost::split(tokens, response, boost::is_any_of(" "));
  is_ack = ((tokens[0] == "ACK") && (tokens[1] == command));

  return is_ack;
}
