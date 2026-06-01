// Copyright 2026 serjimen vja-nie dlesieur
#ifndef INCLUDE_CONFIG_CONFIGPARSER_HPP_
#define INCLUDE_CONFIG_CONFIGPARSER_HPP_

#include <string>
#include <vector>

#include "config/ServerConfig.hpp"

class ConfigParser {
 public:
  ConfigParser();
  explicit ConfigParser(const std::string& filename);
  ConfigParser(const ConfigParser& other);
  ConfigParser& operator=(const ConfigParser& other);
  ~ConfigParser();

  const std::vector<std::string>& getRawLines() const;
  const std::vector<ServerConfig>& getServers() const;

 private:
  std::vector<std::string> _rawLines;
  std::vector<ServerConfig> _servers;

  void parseTokens();
  std::vector<std::string> _flattenTokens() const;
  void _parseServerBlock(const std::vector<std::string>& tokens, size_t* i,
                         ServerConfig* server);
  void _handleLocationDirective(const std::vector<std::string>& tokens,
                                size_t* i, ServerConfig* server);
  void _handleListenDirective(const std::vector<std::string>& tokens, size_t* i,
                              ServerConfig* server);
  void _handleServerNameDirective(const std::vector<std::string>& tokens,
                                  size_t* i, ServerConfig* server);

  void _parseLocationBlock(const std::vector<std::string>& tokens, size_t* i,
                           LocationConfig* location);
  void _handleAllowedMethodsDirective(const std::vector<std::string>& tokens,
                                      size_t* i, LocationConfig* location);
  void _handleCgiPathDirective(const std::vector<std::string>& tokens,
                               size_t* i, LocationConfig* location);
  void _handleCgiExtDirective(const std::vector<std::string>& tokens, size_t* i,
                              LocationConfig* location);
  void _handleRedirectDirective(const std::vector<std::string>& tokens,
                                size_t* i, LocationConfig* location);

  bool _parseContextDirectives(const std::vector<std::string>& tokens,
                               size_t* i, Context* ctx);
  void _handleRootDirective(const std::vector<std::string>& tokens, size_t* i,
                            Context* ctx);
  void _handleIndexDirective(const std::vector<std::string>& tokens, size_t* i,
                             Context* ctx);
  void _handleAutoindexDirective(const std::vector<std::string>& tokens,
                                 size_t* i, Context* ctx);
  void _handleErrorPageDirective(const std::vector<std::string>& tokens,
                                 size_t* i, Context* ctx);
  void _handleClientMaxBodySizeDirective(const std::vector<std::string>& tokens,
                                         size_t* i, Context* ctx);

  static void trimWhitespace(std::string* line);
  static void removeComments(std::string* line);
  static std::vector<std::string> tokenize(const std::string& line);

  // Utils 2
  static void parseHostPort(const std::string& listenVal, std::string* host,
                            std::string* portStr);
  static int validatePort(const std::string& portStr);
  void parseDirective(Context* ctx, const std::string& line);
};

#endif  // INCLUDE_CONFIG_CONFIGPARSER_HPP_
