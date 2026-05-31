/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bbeaurai <bbeaurai@student.42lehavre.fr    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/21 16:01:56 by bbeaurai          #+#    #+#             */
/*   Updated: 2026/05/31 13:02:27 by bbeaurai         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include <unistd.h>
#include "../inc/codexion.h"

static int	all_done(t_data *data)
{
	int	i;

	if (data->nb_compiles_req == 0)
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

	if (data->nb_compiles_req > 0
		&& target->nb_compiles >= data->nb_compiles_req)
		return (0);
	now = current_time_ms();
	if (now - target->last_compile_start > data->time_to_burnout)
	{
		data->sim_running = 0;
		pthread_cond_broadcast(&data->sched_cond);
		pthread_mutex_unlock(&data->sched_lock);
		print_state(data, target->id, "burned out");
		return (1);
	}
	return (0);
}

static int	check_all_burnouts(t_data *data)
{
	int	i;

	i = 0;
	while (i < data->nb_coders)
	{
		if (check_burnout(data, &data->coders[i]))
			return (1);
		i++;
	}
	return (0);
}

static int	monitor_cycle(t_data *data)
{
	int	running;
	int	done;

	pthread_mutex_lock(&data->sched_lock);
	if (!data->sim_running)
	{
		pthread_mutex_unlock(&data->sched_lock);
		return (0);
	}
	if (check_all_burnouts(data))
		return (0);
	done = all_done(data);
	if (done)
		data->sim_running = 0;
	running = data->sim_running;
	pthread_cond_broadcast(&data->sched_cond);
	pthread_mutex_unlock(&data->sched_lock);
	if (done)
	{
		pthread_mutex_lock(&data->print_lock);
		printf("\n\033[0;32mAll coders have compiled.\033[0m\n");
		pthread_mutex_unlock(&data->print_lock);
	}
	return (running);
}

void	*monitor_thread(void *arg)
{
	t_data	*data;

	data = (t_data *)arg;
	while (monitor_cycle(data))
		usleep(100);
	return (NULL);
}
