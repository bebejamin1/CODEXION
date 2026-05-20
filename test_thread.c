/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test_thread.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bbeaurai <bbeaurai@student.42lehavre.fr    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/19 14:17:28 by bbeaurai          #+#    #+#             */
/*   Updated: 2026/05/20 10:54:29 by bbeaurai         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

struct	s_codeur
{
	int	id;
} t_codeur;

void	*routine(void *arg)
{
	printf("Codeur %d est prêt !\n", t_codeur.id);
}

int	main(void)
{
	pthread_t	tid1;
	printf("Le main ce lance\n");
	pthread_create(&tid1, NULL, routine, NULL);
	pthread_join(tid1, NULL);
	return (0);
}
