# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: webserv-42                                 +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/05/20 14:37:00                      #+#    #+#              #
#    Updated: 2026/05/20 14:37:00                     ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

# ──────────────────────────────────────────────────────────────────────────── #
#                              PROJECT CONFIG                                  #
# ──────────────────────────────────────────────────────────────────────────── #

NAME		= webserv

CXX			= c++
CXXFLAGS	= -Wall -Wextra -Werror -std=c++98
INCLUDES	= -Isrc

# ──────────────────────────────────────────────────────────────────────────── #
#                                 SOURCES                                      #
# ──────────────────────────────────────────────────────────────────────────── #

SRC_DIR		= src
OBJ_DIR		= obj

SRCS		= $(SRC_DIR)/main.cpp \
			  $(SRC_DIR)/config/Context.cpp \
			  $(SRC_DIR)/config/LocationConfig.cpp \
			  $(SRC_DIR)/config/ServerConfig.cpp \
			  $(SRC_DIR)/config/ConfigParser.cpp \
			  $(SRC_DIR)/handlers/StaticRouter.cpp \
			  $(SRC_DIR)/handlers/FileHandler.cpp \
			  $(SRC_DIR)/http/HttpRequest.cpp \
			  $(SRC_DIR)/http/HttpResponse.cpp \
			  $(SRC_DIR)/http/RequestParser.cpp \
			  $(SRC_DIR)/network/ListeningSocket.cpp \
			  $(SRC_DIR)/network/ClientSocket.cpp \
			  $(SRC_DIR)/network/EventLoop.cpp

OBJS		= $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

HDRS		= $(SRC_DIR)/config/Context.hpp \
			  $(SRC_DIR)/config/LocationConfig.hpp \
			  $(SRC_DIR)/config/ServerConfig.hpp \
			  $(SRC_DIR)/config/ConfigParser.hpp \
			  $(SRC_DIR)/handlers/StaticRouter.hpp \
			  $(SRC_DIR)/handlers/FileHandler.hpp \
			  $(SRC_DIR)/http/HttpRequest.hpp \
			  $(SRC_DIR)/http/HttpResponse.hpp \
			  $(SRC_DIR)/http/RequestParser.hpp \
			  $(SRC_DIR)/network/ListeningSocket.hpp \
			  $(SRC_DIR)/network/ClientSocket.hpp \
			  $(SRC_DIR)/network/EventLoop.hpp

# ──────────────────────────────────────────────────────────────────────────── #
#                                  TESTS                                       #
# ──────────────────────────────────────────────────────────────────────────── #

TEST_DIR	= tests
BIN_DIR		= bin

# Library sources (everything except main.cpp)
LIB_SRCS	= $(SRC_DIR)/config/Context.cpp \
			  $(SRC_DIR)/config/LocationConfig.cpp \
			  $(SRC_DIR)/config/ServerConfig.cpp \
			  $(SRC_DIR)/config/ConfigParser.cpp \
			  $(SRC_DIR)/handlers/StaticRouter.cpp \
			  $(SRC_DIR)/handlers/FileHandler.cpp \
			  $(SRC_DIR)/http/HttpRequest.cpp \
			  $(SRC_DIR)/http/HttpResponse.cpp \
			  $(SRC_DIR)/http/RequestParser.cpp \
			  $(SRC_DIR)/network/ListeningSocket.cpp \
			  $(SRC_DIR)/network/ClientSocket.cpp \
			  $(SRC_DIR)/network/EventLoop.cpp \
			  $(TEST_DIR)/integration/test_globals.cpp

TEST_SRCS	= $(filter-out $(TEST_DIR)/integration/test_globals.cpp $(TEST_DIR)/integration/stress_client.cpp, $(wildcard $(TEST_DIR)/*/*.cpp))
TEST_NAMES	= $(notdir $(TEST_SRCS:.cpp=))
TEST_BINS	= $(addprefix $(BIN_DIR)/, $(TEST_NAMES))

# ──────────────────────────────────────────────────────────────────────────── #
#                                 COLORS                                       #
# ──────────────────────────────────────────────────────────────────────────── #

ESC			= \033
BOLD		= $(ESC)[1m
RESET		= $(ESC)[0m
GREEN		= $(ESC)[32m
CYAN		= $(ESC)[36m
YELLOW		= $(ESC)[33m
RED			= $(ESC)[31m
MAGENTA		= $(ESC)[35m
BG_GREEN	= $(ESC)[42m
BG_RED		= $(ESC)[41m
DIM			= $(ESC)[2m

# ──────────────────────────────────────────────────────────────────────────── #
#                                 RULES                                        #
# ──────────────────────────────────────────────────────────────────────────── #

all: $(NAME)

$(NAME): $(OBJS)
	@$(CXX) $(CXXFLAGS) $(INCLUDES) $(OBJS) -o $(NAME)
	@echo "$(BOLD)$(GREEN)"
	@echo "  ╔══════════════════════════════════════════╗"
	@echo "  ║      ✅  $(NAME) compiled successfully     ║"
	@echo "  ╚══════════════════════════════════════════╝"
	@echo "$(RESET)"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(HDRS)
	@mkdir -p $(dir $@)
	@echo "  $(CYAN)$(BOLD)⚙$(RESET)  $(DIM)Compiling$(RESET) $<"
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# ──────────────────────────────────────────────────────────────────────────── #
#                              CLEAN RULES                                     #
# ──────────────────────────────────────────────────────────────────────────── #

clean:
	@rm -rf $(OBJ_DIR)
	@echo "  $(YELLOW)$(BOLD)🧹$(RESET)  $(DIM)Removed object files$(RESET)"

fclean: clean
	@rm -f $(NAME)
	@rm -rf $(BIN_DIR)
	@echo "  $(YELLOW)$(BOLD)🗑$(RESET)  $(DIM)Removed $(NAME) and test binaries$(RESET)"

re: fclean all

# ──────────────────────────────────────────────────────────────────────────── #
#                               TEST RULES                                     #
# ──────────────────────────────────────────────────────────────────────────── #

stress: $(BIN_DIR)/stress_client

$(BIN_DIR)/stress_client: $(TEST_DIR)/integration/stress_client.cpp
	@mkdir -p $(BIN_DIR)
	@echo "  $(CYAN)$(BOLD)⚙$(RESET)  $(DIM)Compiling stress client$(RESET) $<"
	@$(CXX) $(CXXFLAGS) $(INCLUDES) $< -o $@

test: $(TEST_BINS)
	@echo ""
	@echo "$(BOLD)$(MAGENTA)━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━$(RESET)"
	@echo "$(BOLD)$(MAGENTA)              🧪  RUNNING TEST BATTERY  🧪               $(RESET)"
	@echo "$(BOLD)$(MAGENTA)━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━$(RESET)"
	@echo ""
	@PASS=0; FAIL=0; TOTAL=0; \
	for test_bin in $(TEST_BINS); do \
		test_name=$$(basename $$test_bin); \
		TOTAL=$$((TOTAL + 1)); \
		printf "  $(CYAN)$(BOLD)▶$(RESET)  Running $(BOLD)%-30s$(RESET) " "$$test_name"; \
		if $$test_bin > /dev/null 2>&1; then \
			echo "$(BG_GREEN)$(BOLD) PASS $(RESET)"; \
			PASS=$$((PASS + 1)); \
		else \
			echo "$(BG_RED)$(BOLD) FAIL $(RESET)"; \
			FAIL=$$((FAIL + 1)); \
		fi; \
	done; \
	echo ""; \
	echo "$(BOLD)$(MAGENTA)━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━$(RESET)"; \
	echo "  $(BOLD)📊 Results: $(GREEN)$$PASS passed$(RESET)$(BOLD) / $(RED)$$FAIL failed$(RESET)$(BOLD) / $$TOTAL total$(RESET)"; \
	echo "$(BOLD)$(MAGENTA)━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━$(RESET)"; \
	echo ""; \
	if [ $$FAIL -gt 0 ]; then \
		echo "  $(RED)$(BOLD)✘  Some tests failed!$(RESET)"; \
		echo ""; \
		exit 1; \
	else \
		echo "  $(GREEN)$(BOLD)✔  All tests passed!$(RESET)"; \
		echo ""; \
	fi

$(BIN_DIR)/%: $(TEST_DIR)/*/*.cpp $(LIB_SRCS) $(HDRS)
	@mkdir -p $(BIN_DIR)
	@echo "  $(CYAN)$(BOLD)⚙$(RESET)  $(DIM)Compiling test$(RESET) $<"
	@# Encontramos el archivo fuente correcto para el binario
	@SRC_FILE=$$(find $(TEST_DIR) -name "$*.cpp" | head -n 1); \
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(LIB_SRCS) $$SRC_FILE -o $@

# ──────────────────────────────────────────────────────────────────────────── #
#                                 PHONY                                        #
# ──────────────────────────────────────────────────────────────────────────── #

.PHONY: all clean fclean re test stress
debug:
	@echo $(TEST_BINS)
