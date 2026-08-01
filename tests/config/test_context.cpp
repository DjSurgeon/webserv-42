// Copyright 2026 serjimen vja-nie dlesieur
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "config/Context.hpp"

int main() {
  Context ctx;

  std::cout << "--- Testing Default Values ---" << std::endl;
  std::cout << "Root: '" << ctx.getRoot() << "'" << std::endl;
  std::cout << "Max Body Size: " << ctx.getClientMaxBodySize() << std::endl;
  std::cout << "Autoindex: " << (ctx.getAutoindex() ? "true" : "false")
            << std::endl;

  std::cout << "\n--- Testing Setters ---" << std::endl;
  ctx.setRoot("/var/www/html");
  ctx.setClientMaxBodySize(2048);
  ctx.setAutoindex(true);
  ctx.addIndexFile("index.html");
  ctx.addIndexFile("index.htm");
  ctx.addErrorPage(404, "/404.html");
  ctx.addErrorPage(500, "/500.html");

  std::cout << "Root: " << ctx.getRoot() << std::endl;
  std::cout << "Max Body Size: " << ctx.getClientMaxBodySize() << std::endl;
  std::cout << "Autoindex: " << (ctx.getAutoindex() ? "true" : "false")
            << std::endl;

  std::cout << "Index Files: ";
  const std::vector<std::string>& idx = ctx.getIndexFiles();
  for (size_t i = 0; i < idx.size(); ++i) {
    std::cout << idx[i] << " ";
  }
  std::cout << std::endl;

  std::cout << "Error Pages:" << std::endl;
  const std::map<int, std::string>& err = ctx.getErrorPages();
  std::map<int, std::string>::const_iterator it;
  for (it = err.begin(); it != err.end(); ++it) {
    std::cout << "  " << it->first << " -> " << it->second << std::endl;
  }

  std::cout << "\n--- Testing Copy Constructor ---" << std::endl;
  Context ctx_copy(ctx);
  std::cout << "Copy Root: " << ctx_copy.getRoot() << std::endl;

  std::cout << "\n--- Testing Assignment Operator ---" << std::endl;
  Context ctx_assign;
  ctx_assign = ctx;
  std::cout << "Assign Root: " << ctx_assign.getRoot() << std::endl;

  std::cout << "\n[OK] No assertions failed. Run with valgrind to check leaks."
            << std::endl;

  return 0;
}
