NAME = codexion
CC = cc
CFLAGS = -Wall -Wextra -Werror -pthread -I inc
SRCS = main.c \
	src/actions.c \
	src/cleanup.c \
	src/init.c \
	src/monitor.c \
	src/routine.c \
	src/scheduler.c \
	src/scheduler_checks.c \
	src/scheduler_queue.c \
	src/scheduler_time.c \
	src/threads.c \
	src/utils.c \
	src/validate.c
OBJDIR = obj
OBJS = $(SRCS:%.c=$(OBJDIR)/%.o)

GREEN   = \033[0;32m
RED     = \033[0;31m
RESET   = \033[0m

all: $(NAME)

$(NAME): $(OBJS)
	@echo " "
	@echo "$(GREEN)Compilation of $(NAME)...$(RESET)"
	$(CC) $(CFLAGS) -o $@ $(OBJS)
	@echo " "
	@echo "$(GREEN)$(NAME) Successfully created!$(RESET)"
	@echo " "

$(OBJDIR)/%.o: %.c
	@echo " "
	@echo "$(GREEN)Compiling $<...$(RESET)"
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@echo " "
	@echo "$(RED)Cleaning up object files...$(RESET)"
	@rm -rf $(OBJDIR)
	@echo " "

fclean: clean
	@echo " "
	@echo "$(RED)Removal of $(NAME)$(RESET)"
	@rm -f $(NAME)
	@echo " "

re: fclean all

.PHONY: all clean fclean re
