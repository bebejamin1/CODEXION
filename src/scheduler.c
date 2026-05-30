/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scheduler.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bbeaurai <bbeaurai@student.42lehavre.fr    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/21 16:00:50 by bbeaurai          #+#    #+#             */
/*   Updated: 2026/05/24 10:25:04 by bbeaurai         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/codexion.h"

static int	try_acquire(t_data *data, t_coder *coder, int my_id)
{
	if (coder_can_compile(coder) && priority_is_clear(data, my_id))
	{
		coder->left_dongle->is_used = 1;
		coder->right_dongle->is_used = 1;
		coder->last_compile_start = current_time_ms();
		sched_dequeue(data, my_id);
		pthread_cond_broadcast(&data->sched_cond);
		pthread_mutex_unlock(&data->sched_lock);
		print_dongle_taken(data, coder->id);
		print_dongle_taken(data, coder->id);
		pthread_mutex_lock(&data->sched_lock);
		return (1);
	}
	return (0);
}

static int	wait_one_coder(t_data *data, int my_id)
{
	sched_dequeue(data, my_id);
	while (data->sim_running)
		pthread_cond_wait(&data->sched_cond, &data->sched_lock);
	pthread_mutex_unlock(&data->sched_lock);
	return (0);
}

static void	wait_scheduler(t_data *data, t_coder *coder)
{
	struct timespec	timeout;
	long long		wake_time;

	wake_time = next_wake_time(coder);
	if (wake_time <= current_time_ms())
		pthread_cond_wait(&data->sched_cond, &data->sched_lock);
	else
	{
		get_timeout(&timeout, wake_time);
		pthread_cond_timedwait(&data->sched_cond,
			&data->sched_lock, &timeout);
	}
}

int	scheduler_request(t_data *data, t_coder *coder)
{
	int				my_id;

	my_id = coder->id;
	pthread_mutex_lock(&data->sched_lock);
	sched_enqueue(data, my_id);
	if (data->nb_coders == 1)
		return (wait_one_coder(data, my_id));
	while (1)
	{
		if (!data->sim_running)
		{
			sched_dequeue(data, my_id);
			pthread_mutex_unlock(&data->sched_lock);
			return (0);
		}
		if (try_acquire(data, coder, my_id))
			break ;
		wait_scheduler(data, coder);
	}
	pthread_mutex_unlock(&data->sched_lock);
	return (1);
}

void	scheduler_release(t_data *data, t_coder *coder)
{
	long long	when;

	when = current_time_ms() + data->dongle_cooldown;
	pthread_mutex_lock(&data->sched_lock);
	coder->left_dongle->is_used = 0;
	coder->left_dongle->available_at = when;
	coder->right_dongle->is_used = 0;
	coder->right_dongle->available_at = when;
	pthread_cond_broadcast(&data->sched_cond);
	pthread_mutex_unlock(&data->sched_lock);
}
