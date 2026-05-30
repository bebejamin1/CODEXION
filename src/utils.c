/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bbeaurai <bbeaurai@student.42lehavre.fr    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/21 16:02:12 by bbeaurai          #+#    #+#             */
/*   Updated: 2026/05/24 10:22:20 by bbeaurai         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <sys/time.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include "../inc/codexion.h"

int	ft_atoi(const char *s)
{
	long	result;
	int		sign;

	result = 0;
	sign = 1;
	if (!s)
		return (0);
	while (*s && (*s == ' ' || (*s >= 9 && *s <= 13)))
		s++;
	if (*s == '+' || *s == '-')
	{
		if (*s == '-')
			sign = -1;
		s++;
	}
	while (*s && isdigit((unsigned char)*s))
	{
		result = result * 10 + (*s - '0');
		s++;
	}
	return ((int)(result * sign));
}

int	is_number(const char *s)
{
	if (!s || *s == '\0')
		return (0);
	if (*s == '+')
		s++;
	if (*s == '\0')
		return (0);
	while (*s)
	{
		if (!isdigit((unsigned char)*s))
			return (0);
		s++;
	}
	return (1);
}

long long	current_time_ms(void)
{
	struct timeval	tv;

	if (gettimeofday(&tv, NULL) != 0)
		return (0);
	return ((long long)tv.tv_sec * 1000LL + tv.tv_usec / 1000LL);
}

void	sleep_ms(int ms)
{
	long long	end;

	end = current_time_ms() + ms;
	while (current_time_ms() < end)
		usleep(500);
}
