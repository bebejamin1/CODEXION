/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cleanup.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bbeaurai <bbeaurai@student.42lehavre.fr    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/24 10:25:00 by bbeaurai          #+#    #+#             */
/*   Updated: 2026/05/24 10:25:04 by bbeaurai         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdlib.h>
#include "../inc/codexion.h"

void	cleanup_system(t_data *data)
{
	if (!data)
		return ;
	pthread_mutex_destroy(&data->print_lock);
	pthread_mutex_destroy(&data->sched_lock);
	pthread_cond_destroy(&data->sched_cond);
	free(data->dongles);
	free(data->coders);
	free(data->wait_queue);
	free(data->wait_order);
}
