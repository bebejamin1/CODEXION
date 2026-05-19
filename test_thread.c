/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test_thread.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bbeaurai <bbeaurai@student.42lehavre.fr    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/19 14:17:28 by bbeaurai          #+#    #+#             */
/*   Updated: 2026/05/19 14:43:13 by bbeaurai         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include <pthread.h>
#include <>

struct	s_codeur
{
	int	id;
} t_codeur;

void	*routine(void *arg)
{
	struct t_codeur = {*arg};
	printf("Codeur %d est prêt !\n", t_codeur.id);
}

int	main(void)
{
	routine("ok");
	return (0);
}
