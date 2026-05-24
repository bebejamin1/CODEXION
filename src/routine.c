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

#include <unistd.h>
#include <stdio.h>
#include "../inc/codexion.h"

void	*coder_routine(void *arg)
{
	t_coder	*coder;
	t_data	*data;

	coder = (t_coder *)arg;
	data = coder->data;
	while (data->sim_running)
	{
		scheduler_request(data, coder);
		coder->last_compile_start = current_time_ms();
		print_state(data, coder->id, "is compiling");
		usleep(data->time_to_compile * 1000);
		coder->nb_compiles++;
		scheduler_release(data, coder);
		print_state(data, coder->id, "is debugging");
		usleep(data->time_to_debug * 1000);
		print_state(data, coder->id, "is refactoring");
		usleep(data->time_to_refactor * 1000);
		if (data->nb_compiles_req > 0
			&& coder->nb_compiles >= data->nb_compiles_req)
			break ;
	}
	return (NULL);
}
