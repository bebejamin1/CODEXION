/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bbeaurai <bbeaurai@student.42lehavre.fr    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/14 11:41:43 by bbeaurai          #+#    #+#             */
/*   Updated: 2026/05/20 10:49:21 by bbeaurai         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NC "\e[0m"
#define YELLOW "\e[1;33m"

int	pthread_create(pthread_t *restrict thread,
		const pthread_attr_t *restrict attr,
		void *(*start_routine)(void *),
		void *restrict arg);

// int	pthread_join(pthread_t thread, void **retval);

// int	pthread_detach(pthread_t thread);

void	*thread_routine(void *data)
{
	pthread_t	tid;

	tid = pthread_self();
	printf("%sThread [%ld]: Le plus grand ennui c'est d'exister "
		"sans vivre.%s\n",
		YELLOW, tid, NC);
	return (NULL);
}

int	main(void)
{
	pthread_t	tid1;
	pthread_t	tid2;

	pthread_create(&tid1, NULL, thread_routine, NULL);
	printf("Main: Creation du premier thread [%ld]\n", tid1);

	pthread_create(&tid2, NULL, thread_routine, NULL);
	printf("Main: Creation du second thread [%ld]\n", tid2);

	pthread_join(tid1, NULL);
	printf("Main: Union du premier thread [%ld]\n", tid1);
	pthread_join(tid2, NULL);
	printf("Main: Union du second thread [%ld]\n", tid2);
	return (0);
}
