/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scheduler_queue.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bbeaurai <bbeaurai@student.42lehavre.fr    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/24 10:25:00 by bbeaurai          #+#    #+#             */
/*   Updated: 2026/05/30 00:00:00 by bbeaurai         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/codexion.h"

int	priority_before(t_data *data, int first_id, int second_id)
{
	long long	first_deadline;
	long long	second_deadline;

	if (data->scheduler_type == 0)
		return (data->wait_order[first_id - 1]
			< data->wait_order[second_id - 1]);
	first_deadline = data->coders[first_id - 1].last_compile_start
		+ data->time_to_burnout;
	second_deadline = data->coders[second_id - 1].last_compile_start
		+ data->time_to_burnout;
	if (first_deadline == second_deadline)
		return (first_id < second_id);
	return (first_deadline < second_deadline);
}

static void	swap_waiters(t_data *data, int first, int second)
{
	int	tmp;

	tmp = data->wait_queue[first];
	data->wait_queue[first] = data->wait_queue[second];
	data->wait_queue[second] = tmp;
}

static void	heap_up(t_data *data, int index)
{
	int	parent;

	while (index > 0)
	{
		parent = (index - 1) / 2;
		if (!priority_before(data, data->wait_queue[index],
				data->wait_queue[parent]))
			return ;
		swap_waiters(data, index, parent);
		index = parent;
	}
}

void	sched_enqueue(t_data *data, int coder_id)
{
	if (data->wait_order[coder_id - 1] != 0
		|| data->wait_count >= data->nb_coders)
		return ;
	data->wait_order[coder_id - 1] = data->next_order;
	data->next_order++;
	data->wait_queue[data->wait_count] = coder_id;
	heap_up(data, data->wait_count);
	data->wait_count++;
}

void	sched_dequeue(t_data *data, int coder_id)
{
	int	i;

	i = 0;
	while (i < data->wait_count && data->wait_queue[i] != coder_id)
		i++;
	if (i == data->wait_count)
		return ;
	data->wait_order[coder_id - 1] = 0;
	data->wait_count--;
	data->wait_queue[i] = data->wait_queue[data->wait_count];
	i = 1;
	while (i < data->wait_count)
	{
		heap_up(data, i);
		i++;
	}
}
