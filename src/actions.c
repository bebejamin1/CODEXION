/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   actions.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bbeaurai <bbeaurai@student.42lehavre.fr    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/21 15:59:09 by bbeaurai          #+#    #+#             */
/*   Updated: 2026/05/21 17:44:01 by bbeaurai         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// (prendre dongle, loguer l'état)

// actions.c :
// Une fonction print_state(t_data *data, int id, char *msg) : Elle verrouille
// print_lock, lit l'heure avec gettimeofday, affiche le timestamp et le
// message, puis déverrouille print_lock. Cela garantit des logs propres.
