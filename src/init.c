/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bbeaurai <bbeaurai@student.42lehavre.fr    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/21 15:59:22 by bbeaurai          #+#    #+#             */
/*   Updated: 2026/05/24 10:25:04 by bbeaurai         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdlib.h>
#include <string.h>
#include "../inc/codexion.h"

static int	init_dongle_and_coder(t_data *data, int i)
{
	data->dongles[i].is_used = 0;
	data->dongles[i].available_at = 0;
	data->coders[i].id = i + 1;
	data->coders[i].nb_compiles = 0;
	data->coders[i].last_compile_start = data->start_time;
	data->coders[i].left_dongle = &data->dongles[i];
	data->coders[i].right_dongle = &data->dongles[(i + 1)
		% data->nb_coders];
	data->coders[i].data = data;
	return (1);
}

static int	allocate_system(t_data *data)
{
	data->dongles = malloc(sizeof(t_dongle) * data->nb_coders);
	if (!data->dongles)
		return (0);
	data->coders = malloc(sizeof(t_coder) * data->nb_coders);
	if (!data->coders)
	{
		free(data->dongles);
		return (0);
	}
	data->wait_queue = malloc(sizeof(int) * data->nb_coders);
	if (!data->wait_queue)
	{
		free(data->coders);
		free(data->dongles);
		return (0);
	}
	memset(data->wait_queue, 0, sizeof(int) * data->nb_coders);
	data->wait_order = malloc(sizeof(int) * data->nb_coders);
	if (!data->wait_order)
	{
		free(data->wait_queue);
		free(data->coders);
		free(data->dongles);
		return (0);
	}
	memset(data->wait_order, 0, sizeof(int) * data->nb_coders);
	return (1);
}

static int	init_sync_objects(t_data *data)
{
	if (pthread_mutex_init(&data->print_lock, NULL) != 0)
		return (0);
	if (pthread_mutex_init(&data->sched_lock, NULL) != 0)
		return (0);
	if (pthread_cond_init(&data->sched_cond, NULL) != 0)
		return (0);
	return (1);
}

static int	init_coders(t_data *data)
{
	int	i;

	i = 0;
	while (i < data->nb_coders)
	{
		if (!init_dongle_and_coder(data, i))
			return (0);
		i++;
	}
	return (1);
}

int	init_system(t_data *data)
{
	if (!allocate_system(data))
		return (0);
	if (!init_sync_objects(data))
	{
		free(data->wait_queue);
		free(data->wait_order);
		free(data->coders);
		free(data->dongles);
		return (0);
	}
	if (!init_coders(data))
	{
		free(data->wait_queue);
		free(data->wait_order);
		free(data->coders);
		free(data->dongles);
		return (0);
	}
	data->wait_count = 0;
	data->next_order = 1;
	data->sim_running = 1;
	return (1);
}
