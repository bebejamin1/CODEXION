/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scheduler_queue.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bbeaurai <bbeaurai@student.42lehavre.fr    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/24 10:25:00 by bbeaurai          #+#    #+#             */
/*   Updated: 2026/05/24 10:25:04 by bbeaurai         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/codexion.h"

static void	enqueue_edf(t_data *data, int coder_id, int *i_ptr)
{
	long long	now;
	long long	rem_new;
	long long	rem;
	int			i;

	now = current_time_ms();
	rem_new = data->coders[coder_id - 1].last_compile_start
		+ data->time_to_burnout - now;
	i = 0;
	while (i < data->wait_count)
	{
		rem = data->coders[data->wait_queue[i] - 1].last_compile_start
			+ data->time_to_burnout - now;
		if (rem > rem_new)
			break ;
		if (rem == rem_new && coder_id < data->wait_queue[i])
			break ;
		i++;
	}
	*i_ptr = i;
}

void	sched_enqueue(t_data *data, int coder_id)
{
	int	i;
	int	j;

	if (data->wait_count >= data->nb_coders)
		return ;
	if (data->scheduler_type == 1)
		enqueue_edf(data, coder_id, &i);
	else
		i = data->wait_count;
	j = data->wait_count;
	while (j > i)
	{
		data->wait_queue[j] = data->wait_queue[j - 1];
		j--;
	}
	data->wait_queue[i] = coder_id;
	data->wait_count++;
}

void	sched_dequeue(t_data *data, int coder_id)
{
	int	i;
	int	j;

	i = 0;
	j = 0;
	while (i < data->wait_count)
	{
		if (data->wait_queue[i] != coder_id)
			data->wait_queue[j++] = data->wait_queue[i];
		i++;
	}
	data->wait_count = j;
}
