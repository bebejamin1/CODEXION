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
	long long	now;
	int			left_free;
	int			right_free;

	now = current_time_ms();
	left_free = (!coder->left_dongle->is_used)
		&& (now >= coder->left_dongle->available_at);
	right_free = (!coder->right_dongle->is_used)
		&& (now >= coder->right_dongle->available_at);
	if (left_free && right_free && data->wait_count > 0
		&& data->wait_queue[0] == my_id)
	{
		coder->left_dongle->is_used = 1;
		coder->right_dongle->is_used = 1;
		sched_dequeue(data, my_id);
		return (1);
	}
	return (0);
}

void	scheduler_request(t_data *data, t_coder *coder)
{
	int	my_id;

	my_id = coder->id;
	pthread_mutex_lock(&data->sched_lock);
	sched_enqueue(data, my_id);
	while (1)
	{
		if (!data->sim_running)
		{
			sched_dequeue(data, my_id);
			pthread_mutex_unlock(&data->sched_lock);
			return ;
		}
		if (try_acquire(data, coder, my_id))
			break ;
		pthread_cond_wait(&data->sched_cond, &data->sched_lock);
	}
	pthread_mutex_unlock(&data->sched_lock);
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
