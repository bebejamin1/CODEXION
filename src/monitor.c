/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bbeaurai <bbeaurai@student.42lehavre.fr    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/21 16:01:56 by bbeaurai          #+#    #+#             */
/*   Updated: 2026/05/21 16:04:42 by bbeaurai         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// Le thread arbitre qui surveille la mort (burnout)

// monitor.c :
// C'est le thread de surveillance exigé par le sujet. Il boucle en continu sur tous les codeurs pour voir si le temps actuel dépasse leur last_compile_start + time_to_burnout. S'il y a un burnout, il met sim_running à 0, affiche le message, et arrête tout.