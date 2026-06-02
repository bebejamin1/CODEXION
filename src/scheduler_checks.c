/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scheduler_checks.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bbeaurai <bbeaurai@student.42lehavre.fr    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/30 00:00:00 by bbeaurai          #+#    #+#             */
/*   Updated: 2026/06/02 17:43:02 by bbeaurai         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/codexion.h"

static int	shares_dongle(t_data *data, int first_id, int second_id)
{
	int	first_left;
	int	first_right;
	int	second_left;
	int	second_right;

	first_left = first_id - 1;
	first_right = first_id % data->nb_coders;
	second_left = second_id - 1;
	second_right = second_id % data->nb_coders;
	return (first_left == second_left || first_left == second_right
		|| first_right == second_left || first_right == second_right);
}

int	coder_can_compile(t_coder *coder)
{
	long long	now;

	now = current_time_ms();
	return (!coder->left_dongle->is_used && !coder->right_dongle->is_used
		&& now >= coder->left_dongle->available_at
		&& now >= coder->right_dongle->available_at);
}

int	priority_is_clear(t_data *data, int my_id)
{
	int	i;
	int	other_id;

	i = 0;
	while (i < data->wait_count)
	{
		other_id = data->wait_queue[i];
		if (shares_dongle(data, my_id, other_id)
			&& priority_before(data, other_id, my_id)
			&& coder_can_compile(&data->coders[other_id - 1]))
			return (0);
		i++;
	}
	return (data->wait_order[my_id - 1] != 0);
}
