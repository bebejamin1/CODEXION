/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   threads.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bbeaurai <bbeaurai@student.42lehavre.fr    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/24 10:25:00 by bbeaurai          #+#    #+#             */
/*   Updated: 2026/05/24 10:25:04 by bbeaurai         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/codexion.h"

static void	enqueue_all(t_data *data)
{
	int	i;

	i = 1;
	while (i <= data->nb_coders)
	{
		sched_enqueue(data, i);
		i += 2;
	}
	i = 2;
	while (i <= data->nb_coders)
	{
		sched_enqueue(data, i);
		i += 2;
	}
}

int	init_threads(t_data *data, pthread_t *monitor)
{
	int	i;

	data->start_time = current_time_ms();
	pthread_mutex_lock(&data->sched_lock);
	i = 0;
	while (i < data->nb_coders)
	{
		data->coders[i].last_compile_start = data->start_time;
		i++;
	}
	enqueue_all(data);
	pthread_mutex_unlock(&data->sched_lock);
	i = 0;
	while (i < data->nb_coders)
	{
		if (pthread_create(&data->coders[i].thread_id, NULL,
				coder_routine, &data->coders[i]) != 0)
			return (0);
		i++;
	}
	if (pthread_create(monitor, NULL, monitor_thread, data) != 0)
		return (0);
	return (1);
}

void	wait_threads(t_data *data, pthread_t monitor)
{
	int	i;

	i = 0;
	while (i < data->nb_coders)
	{
		pthread_join(data->coders[i].thread_id, NULL);
		i++;
	}
	pthread_join(monitor, NULL);
}
