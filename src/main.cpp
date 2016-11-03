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

void usage(const char *prog)
{
	std::cout << "Usage: " << prog
		<< " {GET|POST} <url> <access_token>" << std::endl;
}

int main(int argc, const char *argv[])
{
	using namespace Net;
	using namespace boost::asio;

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
		
		// ASIO service
		io_service svc;

		// Request contents
		std::ostringstream request_stream;
		request_stream << method << " " << path << " HTTP/1.1\r\n"
			<< "Authorization: Bearer " << access_token << "\r\n"
			<< "Accept: */*\r\n"
			<< "Host: " << domain << "\r\n"
			<< "Connection: close\r\n";
		auto request = request_stream.str();

		if(protocol == "http" || protocol == "80") {
			// Simple socket and query
			tcp_client::socket_t socket(svc);
			tcp_client::query_t query(domain, "http");
			tcp_client client(svc, socket, query,
					on_read_headers, on_read_content);
			client << request << std::endl;
			svc.run();
		} else if(protocol == "https" || protocol == "443") {
			// SSL context from system default paths
			ssl::context ctx(ssl::context::sslv23);
			ctx.set_default_verify_paths();

			// Stream from ASIO service and SSL context
			ssl_tcp_client::socket_t socket(svc, ctx);
			socket.set_verify_mode(ssl::verify_peer);
			ssl_tcp_client::query_t query(domain, "https");

			// Client with SSL
			ssl_tcp_client client(svc, socket, query,
					on_read_headers, on_read_content);
			client << request << std::endl;
			svc.run();
		} else {
			std::cerr << "Unknown protocol specified: "
				<< protocol << std::endl;
		}
	} catch(std::exception &ex) {
		std::cout << "Exception: " << ex.what() << std::endl;
	}
}
