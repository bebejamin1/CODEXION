/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scheduler_time.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bbeaurai <bbeaurai@student.42lehavre.fr    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/30 00:00:00 by bbeaurai          #+#    #+#             */
/*   Updated: 2026/05/30 00:00:00 by bbeaurai         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/codexion.h"

void	get_timeout(struct timespec *timeout, long long wake_time)
{
	timeout->tv_sec = wake_time / 1000;
	timeout->tv_nsec = (wake_time % 1000) * 1000000;
}

long long	next_wake_time(t_coder *coder)
{
	long long	left;
	long long	right;

	left = coder->left_dongle->available_at;
	right = coder->right_dongle->available_at;
	if (left > right)
		return (left);
	return (right);
}
