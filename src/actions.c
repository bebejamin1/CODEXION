/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   actions.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bbeaurai <bbeaurai@student.42lehavre.fr    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/21 15:59:09 by bbeaurai          #+#    #+#             */
/*   Updated: 2026/05/24 10:00:48 by bbeaurai         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <pthread.h>
#include <stdio.h>
#include "../inc/codexion.h"

void	print_state(t_data *data, int id, const char *msg)
{
	long long	ts;

	pthread_mutex_lock(&data->print_lock);
	ts = current_time_ms() - data->start_time;
	printf("%lld ms \tCoder %d %s\n", ts, id, msg);
	pthread_mutex_unlock(&data->print_lock);
}
