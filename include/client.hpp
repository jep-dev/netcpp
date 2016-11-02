#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <functional>
#include <map>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

namespace Net {
	using namespace boost::asio;
	using boost::system::error_code;

	// UDP typedefs
	typedef ip::udp::socket udp_socket_t;
	typedef ssl::stream<udp_socket_t> ssl_udp_socket_t;
	typedef ip::udp::resolver udp_resolver_t;
	typedef udp_resolver_t::query udp_query_t;

	// TCP typedefs
	typedef ip::tcp::socket tcp_socket_t;
	typedef ssl::stream<tcp_socket_t> ssl_tcp_socket_t;
	typedef ip::tcp::resolver tcp_resolver_t;
	typedef tcp_resolver_t::query tcp_query_t;

	typedef std::function<void(std::istream&)> fn_istream_t;
	typedef std::ostream& (*fn_ostream_t)(std::ostream&);

	bool get_status(std::istream &is, unsigned int status);

	template<typename PROTOCOL_TYPE, bool USE_SSL = false>
	struct client_t {
		typedef typename std::conditional<USE_SSL,
				ssl::stream<typename PROTOCOL_TYPE::socket>,
				typename PROTOCOL_TYPE::socket
			>::type socket_t;
		typedef typename PROTOCOL_TYPE::resolver resolver_t;
		typedef typename resolver_t::query query_t;

		client_t& operator<<(std::string line);
		client_t& operator<<(fn_ostream_t fn);
		void on_resolve(error_code const& err,
				typename resolver_t::iterator endpt);
		void on_connect(error_code const& err);
		void on_write(error_code const& err);
		void on_read_status(error_code const& err);
		void on_read_header(error_code const& err);
		void on_read_body(error_code const& err);
		client_t(io_service &svc, socket_t &socket, query_t &query,
				fn_istream_t cb_headers, fn_istream_t cb_content);
	protected:
		streambuf request_buffer, response_buffer;
		resolver_t resolver;
		query_t &query;
		socket_t &socket;
		fn_istream_t cb_headers, cb_content;
	};

	// Client typedefs
	template struct client_t<ip::tcp, true>;
	template struct client_t<ip::tcp, false>;
	//template struct client_t<ip::udp, true>;
	//template struct client_t<ip::udp, false>;
	
	typedef client_t<ip::tcp, true> ssl_tcp_client;
	typedef client_t<ip::tcp, false> tcp_client;
	//typedef client_t<ip::udp, true> ssl_udp_client;
	//typedef client_t<ip::udp, false> udp_client;


	/*
	typedef client_t<ssl_socket_t> ssl_client;
	typedef client_t<socket_t> client;*/
}

#endif
