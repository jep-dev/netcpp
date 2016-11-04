#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <functional>
#include <map>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

namespace Net {
	using namespace boost::asio;
	using boost::system::error_code;

	// Typedefs for TCP sockets, resolver, and query
	typedef ip::tcp::socket socket_t;
	typedef ssl::stream<socket_t> ssl_socket_t;
	typedef ip::tcp::resolver resolver_t;
	typedef resolver_t::query query_t;

	// Typedefs for callbacks and functions
	typedef std::function<void(std::istream&)> fn_istream_t;
	typedef std::ostream& (*fn_ostream_t) (std::ostream&);

	/**@brief Extracts the HTTP status code and validates
	 * the status line (must begin with "HTTP/")
	 * @param is The istream from the response buffer
	 * @param status 200 if valid, otherwise 404, 403, etc.
	 * @return True if and only if the status is 200 and version
	 * is in the form "HTTP/" (e.g. HTTP/1.0) */
	bool get_status(std::istream &is, unsigned int status);


	/**@brief A template for an asynchronous network client
	 * @tparam USE_SSL enables SSL for the socket */
	template<bool USE_SSL = false>
	struct client_t {
		/** @brief Socket chosen by USE_SSL conditional */
		typedef typename std::conditional<USE_SSL,
				ssl_socket_t, socket_t>::type socket_type;

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
		client_t(io_service &svc, socket_type &socket, query_t &query,
				fn_istream_t cb_headers, fn_istream_t cb_content);
	protected:
		streambuf request_buffer, response_buffer;
		resolver_t resolver;
		query_t &query;
		socket_type &socket;
		fn_istream_t cb_headers, cb_content;
	};

	// Client typedefs
	template struct client_t<true>;
	template struct client_t<false>;
	
	typedef client_t<true> ssl_client;
	typedef client_t<false> client;
}

#endif
