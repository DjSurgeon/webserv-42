// Copyright 2026 serjimen vja-nie dlesieur
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "config/Context.hpp"

int main() {
  Context ctx;

  std::cout << "--- Testing Default Values ---" << std::endl;
  std::cout << "Root: '" << ctx.get_root() << "'" << std::endl;
  std::cout << "Max Body Size: " << ctx.get_client_max_body_size() << std::endl;
  std::cout << "Autoindex: " << (ctx.get_autoindex() ? "true" : "false")
            << std::endl;

  std::cout << "\n--- Testing Setters ---" << std::endl;
  ctx.set_root("/var/www/html");
  ctx.set_client_max_body_size(2048);
  ctx.set_autoindex(true);
  ctx.add_index_file("index.html");
  ctx.add_index_file("index.htm");
  ctx.add_error_page(404, "/404.html");
  ctx.add_error_page(500, "/500.html");

  std::cout << "Root: " << ctx.get_root() << std::endl;
  std::cout << "Max Body Size: " << ctx.get_client_max_body_size() << std::endl;
  std::cout << "Autoindex: " << (ctx.get_autoindex() ? "true" : "false")
            << std::endl;

  std::cout << "Index Files: ";
  const std::vector<std::string>& idx = ctx.get_index_files();
  for (size_t i = 0; i < idx.size(); ++i) {
    std::cout << idx[i] << " ";
  }
  std::cout << std::endl;

  std::cout << "Error Pages:" << std::endl;
  const std::map<int, std::string>& err = ctx.get_error_pages();
  std::map<int, std::string>::const_iterator it;
  for (it = err.begin(); it != err.end(); ++it) {
    std::cout << "  " << it->first << " -> " << it->second << std::endl;
  }

  std::cout << "\n--- Testing Copy Constructor ---" << std::endl;
  Context ctx_copy(ctx);
  std::cout << "Copy Root: " << ctx_copy.get_root() << std::endl;

  std::cout << "\n--- Testing Assignment Operator ---" << std::endl;
  Context ctx_assign;
  ctx_assign = ctx;
  std::cout << "Assign Root: " << ctx_assign.get_root() << std::endl;

  std::cout << "\n[OK] No assertions failed. Run with valgrind to check leaks."
            << std::endl;

  return 0;
}
