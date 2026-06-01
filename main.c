/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bbeaurai <bbeaurai@student.42lehavre.fr    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/21 15:57:21 by bbeaurai          #+#    #+#             */
/*   Updated: 2026/06/01 15:38:26 by bbeaurai         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "inc/codexion.h"

static void	set_data_from_args(t_data *data, char **argv)
{
	data->nb_coders = ft_atoi(argv[1]);
	data->time_to_burnout = ft_atoi(argv[2]);
	data->time_to_compile = ft_atoi(argv[3]);
	data->time_to_debug = ft_atoi(argv[4]);
	data->time_to_refactor = ft_atoi(argv[5]);
	data->nb_compiles_req = ft_atoi(argv[6]);
	data->dongle_cooldown = ft_atoi(argv[7]);
	if (strcmp(argv[8], "edf") == 0)
		data->scheduler_type = 1;
	else
		data->scheduler_type = 0;
	data->start_time = current_time_ms();
}

static int	start_simulation(t_data *data, pthread_t *monitor)
{
	if (!init_system(data))
	{
		fprintf(stderr, "[ERROR] failed to initialize system\n");
		free(data);
		return (0);
	}
	if (!init_threads(data, monitor))
	{
		fprintf(stderr, "[ERROR] failed to start threads\n");
		cleanup_system(data);
		free(data);
		return (0);
	}
	return (1);
}

int	main(int argc, char **argv)
{
	t_data		*data;
	pthread_t	monitor;

	if (!validate_args(argc, argv))
		return (EXIT_FAILURE);
	data = malloc(sizeof(t_data));
	if (!data)
	{
		fprintf(stderr, "malloc failed\n");
		return (EXIT_FAILURE);
	}
	set_data_from_args(data, argv);
	if (data->nb_coders == 1)
	{
		printf("\033[0;31m0 0 burned out\033[0m");
		free(data);
		return (EXIT_FAILURE);
	}
	if (!start_simulation(data, &monitor))
		return (EXIT_FAILURE);
	wait_threads(data, monitor);
	cleanup_system(data);
	free(data);
	return (EXIT_SUCCESS);
}
