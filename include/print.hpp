#ifndef PRINT_HPP
#define PRINT_HPP

#include <iostream>
#include <string>

/**@brief Removes HTML tags and sections enclosed with non-printing tags
 * (such as <script>)
 * @param html The original HTML content
 * @return The stripped contents */
std::string strip_html(std::string const& html);

/**@brief Prints parallel content in columns
 * @param lhs The left hand side column
 * @param rhs The right hand side column
 * @return The stream with columns inserted */
std::ostream& print(std::string const& lhs, std::string const& rhs);

/**@brief Prints parallel content in columns
 * @param lhs The left hand side column
 * @param rhs The right hand side column
 * @param dest The output stream to return
 * @return The stream with columns inserted */
std::ostream& print(std::string const& lhs, std::string const& rhs,
		std::ostream &dest);

#endif
