#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <functional>
#include <map>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

namespace Net {
	using namespace boost::asio;
	using boost::system::error_code;

	// Typedefs for UDP sockets, resolver, and query
	typedef ip::udp::socket udp_socket_t;
	typedef ssl::stream<udp_socket_t> ssl_udp_socket_t;
	typedef ip::udp::resolver udp_resolver_t;
	typedef udp_resolver_t::query udp_query_t;

	// Typedefs for TCP sockets, resolver, and query
	typedef ip::tcp::socket tcp_socket_t;
	typedef ssl::stream<tcp_socket_t> ssl_tcp_socket_t;
	typedef ip::tcp::resolver tcp_resolver_t;
	typedef tcp_resolver_t::query tcp_query_t;

	// Typedefs for callbacks and functions
	typedef std::function<void(std::istream&)> fn_istream_t;
	typedef std::ostream& (*fn_ostream_t) (std::ostream&);

	/**@brief Extracts the HTTP status code and validates
	 * the status line (must begin with "HTTP/")
	 *
	 * @param is The istream from the response buffer
	 * @param status 200 if valid, otherwise 404, 403, etc.
	 * @return True if and only if the status is 200 and version
	 * is in the form "HTTP/" (e.g. HTTP/1.0)
	 */
	bool get_status(std::istream &is, unsigned int status);


	/**@brief A template for an asynchronous network client
	 * @tparam PROTOCOL either ip::udp or ip::tcp
	 * @tparam USE_SSL enables SSL for the socket
	 */
	template<typename PROTOCOL, bool USE_SSL = false>
	struct client_t {
		/** @brief Socket chosen by USE_SSL conditional */
		typedef typename std::conditional<USE_SSL,
				ssl::stream<typename PROTOCOL::socket>,
				typename PROTOCOL::socket>::type socket_t;

		/** @brief Resolver used to resolve a query to endpoints */
		typedef typename PROTOCOL::resolver resolver_t;
		
		/** @brief Query, containing a host and a protocol */
		typedef typename resolver_t::query query_t;

		/**@brief Stream insertion operator; inserts each line into
		 * the request buffer while preparing a request
		 * @param line The line to insert
		 * @return The same client (to support continuation) */
		client_t& operator<<(std::string line);

		/**@brief Stream insertion operator; inserts the function
		 * and completes the request.
		 * @param fn The function to insert, presumably std::endl
		 * @return The same client (to support continuation) */
		client_t& operator<<(fn_ostream_t fn);

		/**@brief Handles the resolution event and generates a
		 * connection event unless an error occurred during resolution.
		 * @param err The error generated during resolution
		 * @param endpt The resolved endpoint */
		void on_resolve(error_code const& err,
				typename resolver_t::iterator endpt);

		/**@brief Handles the connection event and generates a 
		 * handshake event (if SSL is enabled) and a write event
		 * @param err The error generated while connecting */
		void on_connect(error_code const& err);

		/**@brief Handles the write event and generates a read event
		 * @param err The error generated while  writing */
		void on_write(error_code const& err);

		/**@brief Handles the first read event and generates the next
		 * @param err The error generated while reading the status */
		void on_read_status(error_code const& err);

		/**@brief Handles the second read event and generates the next
		 * @param err The error generated while reading the headers */
		void on_read_header(error_code const& err);

		/**@brief Handles the final read event and recurses
		 * @param err The error generated during */
		void on_read_body(error_code const& err);

		/**@brief Constructor
		 * @param svc reference to an I/O service
		 * @param socket reference to a socket
		 * @param query reference to a query
		 * @param cb_headers consumer for produced headers
		 * @param cb_content handler for produced content */
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
