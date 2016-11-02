#include "net.hpp"
#include <iostream>
#include <boost/bind.hpp>

namespace Net {

	bool get_status(std::istream &is, unsigned int status)
	{
		std::string version, message;
		is >> version >> status;
		std::getline(is, message);
		if(!is) {
			return false;
		}
		if(status != 200) {
			return false;
		}
		if(version.length() < 5 || version.substr(0, 5) != "HTTP/") {
			return false;
		}
		return true;
	}

	template<typename PROTOCOL, bool USE_SSL>
	client_t<PROTOCOL, USE_SSL>::client_t(
			io_service &svc, socket_t &socket, query_t &query,
			fn_istream_t cb_headers, fn_istream_t cb_content):
		resolver(svc), socket(socket), query(query),
		cb_headers(cb_headers), cb_content(cb_content) {}

	template<typename PROTOCOL, bool USE_SSL>
	client_t<PROTOCOL, USE_SSL>&
	client_t<PROTOCOL, USE_SSL>::operator<<(std::string line)
	{
		std::ostream request_stream(&request_buffer);
		request_stream << line;
		return *this;
	}

	template<typename PROTOCOL, bool USE_SSL>
	client_t<PROTOCOL, USE_SSL>&
	client_t<PROTOCOL, USE_SSL>::operator<<(fn_ostream_t fn)
	{
		std::ostream request_stream(&request_buffer);
		request_stream << fn;
		resolver.async_resolve(query,
				boost::bind(&client_t::on_resolve, this,
					placeholders::error, placeholders::iterator));
	}

	template<typename PROTOCOL, bool USE_SSL>
	void client_t<PROTOCOL, USE_SSL>::on_write(error_code const& err)
	{
		if(err) {
			std::cout << "Write error: " << err.message() << std::endl;
			return;
		}
		async_read_until(socket, response_buffer, "\r\n",
				boost::bind(&client_t::on_read_status, this,
					placeholders::error));
	}

	template<typename PROTOCOL, bool USE_SSL>
	void client_t<PROTOCOL, USE_SSL>::on_read_status(error_code const& err)
	{
		if(err) {
			std::cout << "Read status error: " << err.message() << std::endl;
			return;
		}
		unsigned int status;
		std::istream response_stream(&response_buffer);
		if(!get_status(response_stream, status)) {
			std::cout << "Status read error: " << status << std::endl;
			return;
		}
		async_read_until(socket, response_buffer, "\r\n\r\n",
				boost::bind(&client_t::on_read_header, this,
					placeholders::error));
	}

	template<typename PROTOCOL, bool USE_SSL>
	void client_t<PROTOCOL, USE_SSL>::on_read_header(error_code const& err)
	{
		if(err) {
			std::cout << "Read header error: " << err.message() << std::endl;
			return;
		}
		std::istream response_stream(&response_buffer);
		cb_headers(response_stream);
		on_read_body(err);
	}

	template<typename PROTOCOL, bool USE_SSL>
	void client_t<PROTOCOL, USE_SSL>::on_read_body(error_code const& err)
	{
		static const error_code short_read(ERR_PACK(ERR_LIB_SSL,
				0, SSL_R_SHORT_READ), error::get_ssl_category());
		if(err) {
			if(err != short_read && err != error::eof) {
				std::cout << "Read body error: " << err.message() << std::endl;
			}
			return;
		}
		if(response_buffer.size()) {
			std::istream is(&response_buffer);
			cb_content(is);
		}
		async_read(socket, response_buffer, transfer_at_least(1),
				boost::bind(&client_t::on_read_body, this,
					placeholders::error));
	}

	template<typename PROTOCOL, bool USE_SSL>
	void client_t<PROTOCOL, USE_SSL>::on_connect(error_code const& err)
	{
		if(err) {
			std::cout << "Connect error: " << err.message() << std::endl;
			return;
		}
		async_write(socket, request_buffer,
				boost::bind(&client_t::on_write, this,
					placeholders::error));
	}

	template<typename PROTOCOL, bool USE_SSL>
	void client_t<PROTOCOL, USE_SSL>::on_resolve(
			error_code const& err, typename resolver_t::iterator endpt)
	{
		if(err) {
			std::cout << "Resolve error: " << err.message() << std::endl;
			return;
		}
		async_connect(socket.lowest_layer(), endpt,
				boost::bind(&client_t::on_connect, this,
					placeholders::error));
	}

	template<>
	void client_t<ip::tcp, false>::on_connect(error_code const& err)
	{
		if(err) {
			std::cout << "Connect error: " << err.message() << std::endl;
			return;
		}

		async_write(socket, request_buffer,
				boost::bind(&client_t::on_write, this,
					placeholders::error));
	}

	template<>
	void client_t<ip::tcp, true>::on_connect(error_code const& err)
	{
		if(err) {
			std::cout << "Connect error: " << err.message() << std::endl;
			return;
		}
		auto on_handshake = [&] (error_code const& err) {
			if(err) {
				std::cout << "Handshake error: " << err.message() << std::endl;
				return;
			}
			async_write(socket, request_buffer,
					boost::bind(&client_t::on_write, this,
						placeholders::error));
		};
		socket.async_handshake(ssl::stream_base::client, on_handshake);
	}
}
