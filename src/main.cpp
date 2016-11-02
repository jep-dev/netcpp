#include "main.hpp"
#include "net.hpp"
#include "print.hpp"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <map>
#include <sstream>
#include <vector>

#include <boost/asio/ssl.hpp>

int get_protocol(std::string const& url, std::string &protocol, int offset = 0)
{
	auto start = url.begin() + offset, stop = url.end();
	auto sep = url.find("://", offset);
	if(sep == std::string::npos) {
		auto colon = url.find(":", offset),
			 slash = std::min(url.find("/", offset), url.find("?", offset));
		if(colon == std::string::npos) {
			// No protocol, no port
			protocol = "http";
			return offset;
		} else {
			if(slash == std::string::npos) {
				protocol = url.substr(colon + 1);
				return offset;
			} else {
				protocol = url.substr(colon + 1, slash - colon - 1);
				return offset;
			}
		}
	} else {
		protocol = url.substr(offset, sep - offset);
		return offset + protocol.length() + 3;
	}
}

int get_domain(std::string const& url, std::string &domain, int offset = 0)
{
	auto slash = url.find("/", offset),
		 question = url.find("?", offset),
		 colon = url.find(":", offset),
		 end1 = std::min(slash, question),
		 end2 = std::min(end1, colon);
	domain = url.substr(offset, end2 - offset);
	return end1 == std::string::npos ? url.length() : end1;
}

int get_path(std::string const& url, std::string &path, int offset = 0)
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

void usage(const char *prog)
{
	std::cout << "Usage: " << prog
		<< " {GET|POST} <url> <access_token>" << std::endl;
}

int main(int argc, const char *argv[])
{
	if(argc != 4) {
		usage(argv[0]);
		return 1;
	}
	std::string protocol, domain, path,
		method = argv[1], url = argv[2], access_token = argv[3];
	split_url(url, protocol, domain, path);

	// Headers callback (read-and-discard)
	auto on_read_headers = [] (std::istream &is) {
		std::string line;
		while(std::getline(is, line) && line != "\r") {
		}
	};

	// Content callback (read-and-print)
	auto on_read_content = [] (std::istream &is) {
		char c;
		while(is.get(c)) {
			std::cout << c;
		}
		std::cout << std::flush;
	};

	try {
		using namespace boost::asio;
		
		// ASIO service
		io_service svc;

		// Request contents
		std::ostringstream request;
		request << method << " " << path << " HTTP/1.1\r\n"
			<< "Authorization: Bearer " << access_token << "\r\n"
			<< "Accept: */*\r\n"
			<< "Host: " << domain << "\r\n"
			<< "Connection: close\r\n";

		if(protocol == "http" || protocol == "80") {
			// Simple socket and query
			ip::tcp::socket socket(svc);
			ip::tcp::resolver::query query(domain, "http");

			// Client without SSL
			Net::tcp_client client(svc, socket, query,
					on_read_headers, on_read_content);
			client << request.str() << std::endl;
			svc.run();
		} else if(protocol == "https" || protocol == "443") {
			// SSL context from system default paths
			ssl::context ctx(ssl::context::sslv23);
			ctx.set_default_verify_paths();

			// Stream from ASIO service and SSL context
			ssl::stream<ip::tcp::socket> socket(svc, ctx);
			socket.set_verify_mode(ssl::verify_peer);

			// Query (host and protocol)
			ip::tcp::resolver::query query(domain, "https");

			// Client with SSL
			Net::ssl_tcp_client client(svc, socket, query,
					on_read_headers, on_read_content);
			client << request.str() << std::endl;
			svc.run();
		} else {
			std::cerr << "Unknown protocol specified: "
				<< protocol << std::endl;
		}
	} catch(std::exception &ex) {
		std::cout << "Exception: " << ex.what() << std::endl;
	}
}
