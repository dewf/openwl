//
// Created by dang on 10/15/17.
//

#ifndef OPENWL_BOOST_STUFF_H
#define OPENWL_BOOST_STUFF_H

#include <vector>
#include <string>

std::string join_strings(std::vector<std::string> parts, const char *delim);
void replace_all(std::string &source_str, const char *toReplace, const char *replaceWith);
//std::vector<std::string> split_string(const char *source, const char *delim);

#endif //OPENWL_BOOST_STUFF_H
