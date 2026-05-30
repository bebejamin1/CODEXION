/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   routine.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bbeaurai <bbeaurai@student.42lehavre.fr    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/21 15:59:42 by bbeaurai          #+#    #+#             */
/*   Updated: 2026/05/24 10:09:17 by bbeaurai         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/codexion.h"

static void	start_compile(t_data *data, t_coder *coder)
{
	print_state(data, coder->id, "is compiling");
}

static void	count_compile(t_data *data, t_coder *coder)
{
	pthread_mutex_lock(&data->sched_lock);
	coder->nb_compiles++;
	pthread_mutex_unlock(&data->sched_lock);
}

void	*coder_routine(void *arg)
{
	t_coder	*coder;
	t_data	*data;

	coder = (t_coder *)arg;
	data = coder->data;
	while (simulation_is_running(data))
	{
		if (!scheduler_request(data, coder))
			break ;
		start_compile(data, coder);
		sleep_ms(data->time_to_compile);
		count_compile(data, coder);
		scheduler_release(data, coder);
		print_state(data, coder->id, "is debugging");
		sleep_ms(data->time_to_debug);
		print_state(data, coder->id, "is refactoring");
		sleep_ms(data->time_to_refactor);
	}
	return (NULL);
}
