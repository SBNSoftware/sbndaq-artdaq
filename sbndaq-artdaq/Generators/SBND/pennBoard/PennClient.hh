/*
 * PennClient.hh
 *
 *  Created on: Dec 15, 2014
 *      Author: tdealtry (based on tcn45 rce code)
 */

#ifndef PENNCLIENT_HH_
#define PENNCLIENT_HH_

#include <string>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/array.hpp>
#include <boost/thread.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/lexical_cast.hpp>
#include <atomic>

#include "sbndaq-artdaq/Generators/SBND/pennBoard/PennCompileOptions.hh"
#include "messagefacility/MessageLogger/MessageLogger.h"

using boost::asio::ip::tcp;

namespace sbnddaq {

	class PennClient
	{
	public:
		PennClient(const std::string& host_name, const std::string& port_or_service, const unsigned int timeout_usecs);
		virtual ~PennClient();

		void send_command(std::string const & command);
		void send_config(std::string const & config);
	  void send_xml(std::string const & xml_frag);
		template<class T> void set_param(std::string const & name, T const & value, std::string const & );

		bool exception() const {return exception_.load();};
	private:

		std::size_t send(std::string const & send_str);
		std::string receive(void);
		void set_param_(std::string const& name, std::string const & encoded_value, std::string const & type);
		bool response_is_ack(std::string const & response, std::string const & command);

		void set_deadline(void);
		void check_deadline(void);
		static void async_connect_handler(const boost::system::error_code& ec, boost::system::error_code* output_ec);
		static void async_completion_handler(
				const boost::system::error_code &error_code, std::size_t length,
				boost::system::error_code* output_error_code, std::size_t* output_length);

    void set_exception( bool exception ) { exception_.store( exception ); }

		boost::asio::io_service     io_service_;
		tcp::socket                 socket_;
		boost::asio::deadline_timer deadline_;
		unsigned int                timeout_usecs_;
		std::atomic<bool>           exception_;
	};

	template<class T>
		void PennClient::set_param(std::string const & name, T const & value, std::string const & type)
	{
		set_param_(name, boost::lexical_cast<std::string>(value), type);
	}

}


#endif /* PENNCLIENT_HH_ */
