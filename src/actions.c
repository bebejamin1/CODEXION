/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   actions.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bbeaurai <bbeaurai@student.42lehavre.fr    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/21 15:59:09 by bbeaurai          #+#    #+#             */
/*   Updated: 2026/05/30 11:25:38 by bbeaurai         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include "../inc/codexion.h"

void	print_state(t_data *data, int id, const char *msg)
{
	long long	ts;

	pthread_mutex_lock(&data->print_lock);
	ts = current_time_ms() - data->start_time;
	if (simulation_is_running(data) || msg[0] == 'b')
	{
		if (msg[0] == 'b')
			printf("\033[0;31m%lld %d %s\033[0m\n", ts, id, msg);
		else
			printf("%lld %d %s\n", ts, id, msg);
		fflush(stdout);
	}
	pthread_mutex_unlock(&data->print_lock);
}

void	print_dongle_taken(t_data *data, int id)
{
	long long	ts;

	pthread_mutex_lock(&data->print_lock);
	ts = current_time_ms() - data->start_time;
	if (simulation_is_running(data))
	{
		printf("%lld %d has taken a dongle\n", ts, id);
		fflush(stdout);
	}
	pthread_mutex_unlock(&data->print_lock);
}

void	print_compile(t_data *data, int id)
{
	long long	ts;

	pthread_mutex_lock(&data->print_lock);
	ts = current_time_ms() - data->start_time;
	if (simulation_is_running(data))
	{
		printf("%lld %d is compiling\n", ts, id);
		fflush(stdout);
	}
	pthread_mutex_unlock(&data->print_lock);
}

void	stop_simulation(t_data *data)
{
	pthread_mutex_lock(&data->sched_lock);
	data->sim_running = 0;
	pthread_cond_broadcast(&data->sched_cond);
	pthread_mutex_unlock(&data->sched_lock);
}

int	simulation_is_running(t_data *data)
{
	int	running;

	pthread_mutex_lock(&data->sched_lock);
	running = data->sim_running;
	pthread_mutex_unlock(&data->sched_lock);
	return (running);
}
