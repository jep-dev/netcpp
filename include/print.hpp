#ifndef PRINT_HPP
#define PRINT_HPP

#include <iostream>
#include <string>

std::string strip_html(std::string const& html);

std::ostream& print(std::string const& lhs, std::string const& rhs);
std::ostream& print(std::string const& lhs, std::string const& rhs,
		std::ostream &dest);

#endif
