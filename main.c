/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bbeaurai <bbeaurai@student.42lehavre.fr    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/21 15:57:21 by bbeaurai          #+#    #+#             */
/*   Updated: 2026/05/24 10:25:03 by bbeaurai         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <pthread.h>
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
		perror("malloc");
		return (EXIT_FAILURE);
	}
	set_data_from_args(data, argv);
	if (!start_simulation(data, &monitor))
		return (EXIT_FAILURE);
	printf("Parsed: coders=%d burnout=%d compile=%d debug=%d refactor=%d\n",
		data->nb_coders, data->time_to_burnout, data->time_to_compile,
		data->time_to_debug, data->time_to_refactor);
	printf("req=%d cooldown=%d scheduler=%s\n",
		data->nb_compiles_req, data->dongle_cooldown, argv[8]);
	wait_threads(data, monitor);
	cleanup_system(data);
	free(data);
	return (EXIT_SUCCESS);
}
