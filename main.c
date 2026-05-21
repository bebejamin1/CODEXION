/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bbeaurai <bbeaurai@student.42lehavre.fr    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/21 15:57:21 by bbeaurai          #+#    #+#             */
/*   Updated: 2026/05/21 16:04:18 by bbeaurai         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// parsing et nettoyage

// number_of_coders time_to_burnout time_to_compile time_to_debug
// time_to_refactor number_of_compiles_required dongle_cooldown scheduler

// main.c : * Vérifie les arguments (refuse les nombres négatifs, gère le texte "fifo" et "edf").  Appelle les fonctions d'initialisation.Lance les threads.Appelle la fonction de nettoyage (destruction de tous les mutex et libération de la mémoire ) à la fin.  