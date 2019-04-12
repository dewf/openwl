//
// Created by dang on 10/15/17.
//

#include "boost_stuff.h"
#include <sstream>
#include <iostream>

//#include <boost/algorithm/string/join.hpp>
//#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string_regex.hpp>

using namespace std;

string join_strings(vector<string> parts, const char *delim)
{
    return boost::algorithm::join(parts, delim);
}

void replace_all(string &source_str, const char *toReplace, const char *replaceWith) {
    return boost::replace_all(source_str, toReplace, replaceWith);
}

//vector<string> split_string(const char *source, const char *delim)
//{
//    vector<string> result;
//    boost::algorithm::split_regex(result, source, boost::regex(delim)) ;
//    return result;
//}
