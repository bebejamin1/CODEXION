/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   routine.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bbeaurai <bbeaurai@student.42lehavre.fr    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/21 15:59:42 by bbeaurai          #+#    #+#             */
/*   Updated: 2026/05/21 17:44:24 by bbeaurai         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// (compile, debug, refactor)

// routine.c :
// Contient la fonction void *coder_routine(void *arg).
// C'est une boucle while (data->sim_running). Le codeur demande ses dongles
// via le scheduler, fait un usleep pour le temps de compilation, rend les 
// dongles, fait un usleep pour le debug, puis un usleep pour le refactor, 
// et recommence.
