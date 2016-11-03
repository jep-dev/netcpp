#ifndef NET_HPP
#define NET_HPP

#include <string>
#include <boost/asio.hpp>

namespace Net {
	int get_protocol(std::string const&, std::string &, int = 0);
	int get_domain(std::string const&, std::string &, int = 0);
	int get_path(std::string const&, std::string &, int = 0);
	void split_url(std::string const&, std::string &,
			std::string &, std::string &);
	template<typename PROTOCOL, bool USE_SSL> struct client_t;
}

#include "client.hpp"

#endif
