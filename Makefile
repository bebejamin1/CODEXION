NAME = codexion
CC = cc
CFLAGS = -Wall -Wextra -Werror -pthread -I inc
SRCS = main.c $(wildcard src/*.c)
OBJDIR = obj
OBJS = $(SRCS:%.c=$(OBJDIR)/%.o)

GREEN   = \033[0;32m
RED     = \033[0;31m
RESET   = \033[0m

all: $(NAME)

$(NAME): $(OBJS)
	@echo " "
	@echo "$(GREEN)🚧 Compilation of $(NAME)...$(RESET)"
	$(CC) $(CFLAGS) -o $@ $(OBJS)
	@echo " "
	@echo "$(GREEN)👌 $(NAME) Successfully created!$(RESET)"
	@echo " "

$(OBJDIR)/%.o: %.c
	@echo " "
	@echo "$(GREEN)⏳ Compilation of $<...$(RESET)"
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	clear
	@echo " "
	@echo "$(RED)🧹 Cleaning up object files...$(RESET)"
	rm -rf $(OBJDIR)
	rm -rf $(NAME)


fclean: clean
	clear
	@echo " "
	@echo "$(RED)🗑️  Removal of $(NAME)$(RESET)"
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re test

test: all
	./tests/run_tests.sh
