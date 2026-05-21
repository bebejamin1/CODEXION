/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bbeaurai <bbeaurai@student.42lehavre.fr    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/21 15:59:22 by bbeaurai          #+#    #+#             */
/*   Updated: 2026/05/21 17:44:06 by bbeaurai         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// init.c : * init_data : Remplit les arguments dans t_data.

// init_dongles : Fait une boucle pour pthread_mutex_init et pthread_cond_init
// sur chaque dongle.

// init_coders : Assigne les bons pointeurs left_dongle et right_dongle
// (attention au dernier codeur qui doit boucler sur le premier dongle ).
