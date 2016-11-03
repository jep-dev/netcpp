#include "net.hpp"

namespace Net {
	int get_protocol(std::string const& url,
			std::string &protocol, int offset)
	{
		auto start = url.begin() + offset, stop = url.end();
		auto sep = url.find("://", offset);
		if(sep == std::string::npos) {
			auto colon = url.find(":", offset),
			 	 slash = std::min(url.find("/", offset),
						 url.find("?", offset));
			if(colon == std::string::npos) {
				// No protocol, no port
				protocol = "http";
				return offset;
			} else {
				colon++;
				protocol = (slash == std::string::npos) ? url.substr(colon)
					: url.substr(colon, slash - colon);
				return offset;
			}
		} else {
			protocol = url.substr(offset, sep - offset);
			return offset + protocol.length() + 3;
		}
	}

	int get_domain(std::string const& url,
			std::string &domain, int offset)
	{
		auto slash = url.find("/", offset),
		 	 question = url.find("?", offset),
		 	 colon = url.find(":", offset),
		 	 end1 = std::min(slash, question),
		 	 end2 = std::min(end1, colon);
		domain = url.substr(offset, end2 - offset);
		return end1 == std::string::npos ? url.length() : end1;
	}

	int get_path(std::string const& url,
			std::string &path, int offset)
	{
		path = offset == url.length() ? "/" : url.substr(offset);
		return offset + path.length();
	}

	void split_url(std::string const& url, std::string &protocol,
			std::string &domain, std::string &path)
	{
		int domain_start = get_protocol(url, protocol),
			path_start = get_domain(url, domain, domain_start),
			path_stop = get_path(url, path, path_start);
	}
}
