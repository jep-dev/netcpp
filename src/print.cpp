#include "print.hpp"
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <vector>

std::string strip_html(std::string const& html) {
	std::string output, tag;
	bool open_tag = false;
	std::vector<std::vector<std::string>> tags;
	std::vector<std::pair<std::string, std::string>> replacements = {
		{"br", "\r\n"}, {"li", "* "}
	};
	std::vector<std::string> suppressed = {
		"head", "script", "style"
	}, noclose = {
		"br", "img", "meta"
	}, blocks = {
		"h1", "h2", "h3", "table", "tr", "p", "li", "pre"
	};
	auto is_noclose = [&noclose] (std::string const& tag) -> bool {
		if(!tag.length() || (tag.back() == '/'))
			return true;
		std::string word;
		std::istringstream iss(tag);
		auto head = std::begin(noclose), tail = std::end(noclose);
		return iss >> word && (std::find(head, tail, word) != tail);
	};
	auto is_replaced = [&replacements] (std::string const& tag,
			std::string &repl) -> bool {
		std::string word;
		std::istringstream iss(tag);
		if(iss >> word) {
			for(auto replacement : replacements) {
				if(word == replacement.first) {
					repl = replacement.second;
					return true;
				}
			}
		}
		return false;
	};
	auto is_block = [&blocks] (std::string const& tag) -> bool {
		auto start = std::begin(blocks), stop = std::end(blocks);
		return std::find(start, stop, tag) != stop;
	};
	auto do_suppress_whitespace = [&tags] (void) -> bool {
		for(auto tag : tags) {
			if(tag.size()) {
				auto word0 = tag[0];
				if(word0 == "pre") {
					return false;
				}
			}
		}
		return true;
	};
	auto do_suppress_all = [&tags, &suppressed] (void) -> bool {
		for(auto tag : tags) {
			if(tag.size()) {
				auto word0 = tag[0];
				for(auto s_it : suppressed) {
					if(word0 == s_it) {
						return true;
					}
				}
			}
		}
		return false;
	};
	bool suppress_whitespace = true, suppress_all = false;
	for(auto c : html) {
		if(c == '<') {
			open_tag = true;
			tag = "";
		} else if(c == '>') {
			open_tag = false;
			if(tag.front() == '/') {
				tag = tag.substr(1);
				while(tags.size()) {
					auto back = tags.back();
					if(back.size()) {
						auto word0 = back[0];
						if(is_block(word0)) {
							output += "\r\n";
						}
						tags.pop_back();
						if(word0 == tag) {
							break;
						}
					}
				}
			} else {
				std::string repl;
				bool found_replacement = is_replaced(tag, repl),
					 found_noclose = is_noclose(tag);
				if(found_replacement) {
					output += repl;
				}
				if(!found_noclose) {
					std::string word;
					std::istringstream iss(tag);
					std::vector<std::string> words;
					while(iss >> word) {
						words.emplace_back(word);
					}
					tags.emplace_back(words);
				}
			}
			suppress_whitespace = do_suppress_whitespace();
			suppress_all = do_suppress_all();
			/*
			for(auto tag : tags) {
				if(tag.size()) {
					auto word0 = tag[0];
					for(auto s_it : suppressed) {
						if(word0 == s_it) {
							suppress = true;
							break;
						}
					}
				}
				if(suppress) break;
			}*/
		} else if(open_tag) {
			tag += c;
		} else if(!suppress_all) {
			if(suppress_whitespace) {
				if((c != '\r') && (c != '\n') && (c != '\t'))
					output += c;
			} else {
				output += c;
			}
		}
	}
	return output;
}


std::ostream& print(std::string const& lhs, std::string const& rhs,
		std::ostream &os)
{
	int lwidth = 40, rwidth = 40;
	std::string border_left = "| ",
		border_center = " | ",
		border_right = " |";

	// Create a stream from each string
	std::istringstream iss_lhs(lhs), iss_rhs(rhs);
	std::vector<std::string> lines_lhs, lines_rhs;
	std::string line;
	
	auto strip = [] (std::string const& src) -> std::string {
		std::string dest;
		for(auto c : src) {
			if(c != '\r')
				dest += c;
		}
		return dest;
	};
	// Add each line to the corresponding vector
	while(std::getline(iss_lhs, line))
		lines_lhs.emplace_back(strip(line));
	while(std::getline(iss_rhs, line))
		lines_rhs.emplace_back(strip(line));

	// For each line in the vectors
	auto end_lhs = std::end(lines_lhs), end_rhs = std::end(lines_rhs);
	for(auto l_it = std::begin(lines_lhs), r_it = std::begin(lines_rhs);
			l_it != end_lhs || r_it != end_rhs;) {
		std::string line_lhs = l_it == end_lhs ? "" : *l_it,
			line_rhs = r_it == end_rhs ? "" : *r_it;
		if(l_it != end_lhs) l_it++;
		if(r_it != end_rhs) r_it++;
		bool once = true;
		while(once || line_lhs.length() || line_rhs.length()) {
			once = false;
			os << border_left;
			if(line_lhs.length() > lwidth) {
				os << line_lhs.substr(0, lwidth);
				line_lhs = line_lhs.substr(lwidth);
			} else {
				os << std::setw(lwidth) << std::right << line_lhs;
				line_lhs = "";
			}
			os << border_center;
			if(line_rhs.length() > rwidth) {
				os << line_rhs.substr(0, rwidth);
				line_rhs = line_rhs.substr(rwidth);
			} else {
				os << std::setw(rwidth) << std::left << line_rhs;
				line_rhs = "";
			}
			os << border_right << std::endl;
		}
	}
}
std::ostream& print(std::string const& lhs, std::string const& rhs)
{
	return print(lhs, rhs, std::cout);
}
