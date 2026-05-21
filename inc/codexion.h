/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bbeaurai <bbeaurai@student.42lehavre.fr    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/21 16:01:27 by bbeaurai          #+#    #+#             */
/*   Updated: 2026/05/21 17:48:29 by bbeaurai         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# include <pthread.h>
# include <sys/time.h>

// Structure pour un Dongle (Ressource partagée)
typedef struct s_dongle
{
    pthread_mutex_t lock;       // Protège l'accès à ce dongle spécifique
    pthread_cond_t  cond;       // Pour endormir les codeurs attendent ce dongle
    int             is_used;    // 0 = libre, 1 = pris
    long long       available_at; // Timestamp pour gérer le dongle_cooldown
}   t_dongle;

// Structure pour un Codeur (Le Thread)
typedef struct s_coder
{
    int             id;
    pthread_t       thread_id;
    int             nb_compiles;
    long long       last_compile_start;
    t_dongle        *left_dongle;  // Pointeur vers le dongle à sa gauche
    t_dongle        *right_dongle; // Pointeur vers le dongle à sa droite
    struct s_data   *data;         // Pointeur structure globale aux paramètres
}   t_coder;

// La "Super StructurSe" globale (à instancier UNE seule fois dans le main)
typedef struct s_data
{
    int             nb_coders;
    int             time_to_burnout;
    int             time_to_compile;
    int             time_to_debug;
    int             time_to_refactor;
    int             nb_compiles_req;
    int             dongle_cooldown;
    int             scheduler_type;
    
    long long       start_time;     // Heure de début de la simulation
    int             sim_running;    // 1 = en cours, 0 = arrêt (burnout ou fin)

    pthread_mutex_t print_lock;     // éviter que les printf se mélangent
    pthread_mutex_t state_lock;     // protége accès à 'sim_running' var du moni

    t_dongle        *dongles;       // Tableau de tous les dongles
    t_coder         *coders;        // Tableau de tous les codeurs
}   t_data;

#endif