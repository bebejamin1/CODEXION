/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bbeaurai <bbeaurai@student.42lehavre.fr    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/21 16:01:56 by bbeaurai          #+#    #+#             */
/*   Updated: 2026/05/24 10:12:20 by bbeaurai         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include <stdio.h>
#include "../inc/codexion.h"

static int	all_done(t_data *data)
{
	int	i;

	if (data->nb_compiles_req <= 0)
		return (0);
	i = 0;
	while (i < data->nb_coders)
	{
		if (data->coders[i].nb_compiles < data->nb_compiles_req)
			return (0);
		i++;
	}
	return (1);
}

static int	check_burnout(t_data *data, t_coder *target)
{
	long long	now;

	now = current_time_ms();
	if (now - target->last_compile_start > data->time_to_burnout)
	{
		print_state(data, target->id, "burnout");
		pthread_mutex_lock(&data->sched_lock);
		data->sim_running = 0;
		pthread_cond_broadcast(&data->sched_cond);
		pthread_mutex_unlock(&data->sched_lock);
		return (1);
	}
	return (0);
}

void	*monitor_thread(void *arg)
{
	t_data	*data;
	int		i;

	data = (t_data *)arg;
	while (data->sim_running)
	{
		i = 0;
		while (i < data->nb_coders)
		{
			if (check_burnout(data, &data->coders[i]))
				return (NULL);
			i++;
		}
		if (all_done(data))
		{
			data->sim_running = 0;
			return (NULL);
		}
		usleep(1000);
	}
	return (NULL);
}
