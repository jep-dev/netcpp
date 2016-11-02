#ifndef NET_HPP
#define NET_HPP

#include <string>
#include <boost/asio.hpp>

namespace Net {
	template<typename PROTOCOL, bool USE_SSL> struct client_t;
}

#include "client.hpp"

#endif
