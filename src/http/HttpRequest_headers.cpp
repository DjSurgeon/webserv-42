// Copyright 2026 serjimen vja-nie dlesieur
#include "http/HttpRequest.hpp"

#include <algorithm>
#include <cctype>
#include <map>
#include <sstream>
#include <string>
#include <vector>

/**
 * @brief Helper used to sort LanguageWeights by their 'q' factor descending.
 * 
 * @param a First language weight.
 * @param b Second language weight.
 * @return true if 'a' has a higher quality factor than 'b'.
 */
static bool compare_language_weight(const LanguageWeight& a, const LanguageWeight& b) {
  return a.q > b.q;
}

/**
 * @brief Removes leading and trailing whitespaces and tabs from a string.
 * 
 * @param str The original string.
 * @return std::string The trimmed string.
 */
static std::string trim_spaces(const std::string& str) {
  size_t start = 0;
  while (start < str.length() && (str[start] == ' ' || str[start] == '\t')) {
    start++;
  }
  size_t end = str.length();
  while (end > start && (str[end - 1] == ' ' || str[end - 1] == '\t')) {
    end--;
  }
  return str.substr(start, end - start);
}

/**
 * @brief Converts a string to lowercase.
 * 
 * @param str The original string.
 * @return std::string The lowercase string.
 */
static std::string to_lower(const std::string& str) {
  std::string lower = str;
  for (size_t i = 0; i < lower.length(); ++i) {
    lower[i] = std::tolower(static_cast<unsigned char>(lower[i]));
  }
  return lower;
}

/**
 * @brief Parses the Accept-Language header into a sorted vector of LanguageWeight structs.
 * 
 * Extracts languages and their respective 'q' values, sorting them from highest
 * to lowest priority. If no 'q' value is specified, it defaults to 1.0.
 * 
 * @return std::vector<LanguageWeight> Sorted vector of accepted languages.
 */
std::vector<LanguageWeight> HttpRequest::get_accepted_languages() const {
  std::vector<LanguageWeight> languages;
  std::map<std::string, std::string>::const_iterator it = _headers.find("accept-language");
  if (it == _headers.end()) {
    return languages;
  }

  std::string raw = it->second;
  size_t start = 0;
  while (start < raw.length()) {
    size_t end = raw.find(',', start);
    if (end == std::string::npos) {
      end = raw.length();
    }
    std::string chunk = trim_spaces(raw.substr(start, end - start));

    if (!chunk.empty()) {
      size_t semi_pos = chunk.find(';');
      LanguageWeight lw;
      if (semi_pos != std::string::npos) {
        lw.lang = trim_spaces(chunk.substr(0, semi_pos));
        std::string q_part = chunk.substr(semi_pos + 1);
        size_t q_pos = q_part.find("q=");
        if (q_pos != std::string::npos) {
          std::stringstream ss(q_part.substr(q_pos + 2));
          ss >> lw.q;
        } else {
          lw.q = 1.0;
        }
      } else {
        lw.lang = chunk;
        lw.q = 1.0;
      }
      languages.push_back(lw);
    }
    start = end + 1;
  }

  std::stable_sort(languages.begin(), languages.end(), compare_language_weight);
  return languages;
}

/**
 * @brief Parses a raw Cookie string and extracts individual key-value pairs.
 * 
 * Populates the internal _cookies map, handling multiple cookies separated by ';'.
 * 
 * @param raw_cookies The raw string from the Cookie header.
 */
void HttpRequest::_parse_cookies_string(const std::string& raw_cookies) {
  size_t start = 0;
  while (start < raw_cookies.length()) {
    size_t end = raw_cookies.find(';', start);
    if (end == std::string::npos) {
      end = raw_cookies.length();
    }
    std::string pair = raw_cookies.substr(start, end - start);
    size_t eq_pos = pair.find('=');
    if (eq_pos != std::string::npos) {
      std::string key = trim_spaces(pair.substr(0, eq_pos));
      std::string value = trim_spaces(pair.substr(eq_pos + 1));
      if (!key.empty()) {
        _cookies[key] = value;
      }
    }
    start = end + 1;
  }
}

/**
 * @brief Adds a header to the request. Automatically parses cookies if the key is 'Cookie'.
 * 
 * @param key The header name.
 * @param value The header value.
 */
void HttpRequest::add_header(const std::string& key, const std::string& value) {
  std::string lower_key = to_lower(key);
  _headers[lower_key] = value;
  if (lower_key == "cookie") {
    _parse_cookies_string(value);
  }
}

/**
 * @brief Explicitly adds a cookie to the request's internal map.
 * 
 * @param key The cookie name.
 * @param value The cookie value.
 */
void HttpRequest::add_cookie(const std::string& key, const std::string& value) {
  _cookies[key] = value;
}
