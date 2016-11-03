#ifndef NET_HPP
#define NET_HPP

#include <string>
#include <boost/asio.hpp>

namespace Net {
	/**@brief Extracts the protocol from the url and returns the offset
	 * of the domain
	 * @param url The original URL
	 * @param protocol The destination for the extracted protocol/port
	 * @param offset The original offset into the url
	 * @return The offset of the domain */
	int get_protocol(std::string const& url,
			std::string & protocol, int offset = 0);

	/**@brief Extracts the domain from the url and returns the offset
	 * of the path
	 * @param url The original URL
	 * @param domain The destination for the extracted domain
	 * @param offset The original offset into the url
	 * @return The offset of the path */
	int get_domain(std::string const& url,
			std::string & domain, int offset = 0);

	/**@brief Extracts the path from the url and returns the offset
	 * of the end of the path
	 * @param url The original URL
	 * @param path The destination for the extracted path
	 * @param offset The original offset into the url
	 * @return The offset of the end of the path */
	int get_path(std::string const& url,
			std::string & path, int offset = 0);

	/**@brief Extracts the protocol, domain, and path from the URL
	 * @param url The original URL
	 * @param protocol The destination for the extracted protocol
	 * @param domain The destination for the extracted domain
	 * @param path The destination for the extracted path */
	void split_url(std::string const& url, std::string & protocol,
			std::string & domain, std::string & path);

	template<typename PROTOCOL, bool USE_SSL> struct client_t;
}

#include "client.hpp"

#endif
