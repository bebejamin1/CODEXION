/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   validate.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bbeaurai <bbeaurai@student.42lehavre.fr    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/24 10:25:00 by bbeaurai          #+#    #+#             */
/*   Updated: 2026/06/03 08:53:09 by bbeaurai         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include <string.h>
#include "../inc/codexion.h"

static void	print_int(void)
{
	printf("The argument passed must be a positive integer.\n");
}

static void	print_scheduler(void)
{
	printf("The scheduler must be fifo or edf.\n");
}

static void	print_usage(void)
{
	printf("Usage: ./codexion number_of_coders time_to_burnout ");
	printf("time_to_compile time_to_debug time_to_refactor ");
	printf("number_of_compiles_required dongle_cooldown scheduler(fifo|edf)\n");
	printf("\ntry with this");
	printf("\n./codexion 2 1000 200 200 200 1 200 edf\n");
}

int	validate_args(int argc, char **argv)
{
	int	i;

	if (argc != 9)
	{
		print_usage();
		return (0);
	}
	i = 1;
	if (ft_atoi(argv[1]) > 20)
		return (printf("The coder number cannot exceed 20"), 0);
	while (i <= 7)
	{
		if (!is_number(argv[i]) || ft_atoi(argv[i]) <= 0)
		{
			print_int();
			return (0);
		}
		i++;
	}
	if (strcmp(argv[8], "fifo") != 0 && strcmp(argv[8], "edf") != 0)
	{
		print_scheduler();
		return (0);
	}
	return (1);
}
