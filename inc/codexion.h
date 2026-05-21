/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bbeaurai <bbeaurai@student.42lehavre.fr    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/21 16:01:27 by bbeaurai          #+#    #+#             */
/*   Updated: 2026/05/21 18:03:00 by bbeaurai         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# include <pthread.h>

typedef struct s_dongle
{
	pthread_mutex_t	lock;
	pthread_cond_t	cond;
	int				is_used;
	long long		available_at;
}	t_dongle;

typedef struct s_coder
{
	int				id;
	pthread_t		thread_id;
	int				nb_compiles;
	long long		last_compile_start;
	t_dongle		*left_dongle;
	t_dongle		*right_dongle;
	struct s_data	*data;
}	t_coder;

typedef struct s_data
{
	int				nb_coders;
	int				time_to_burnout;
	int				time_to_compile;
	int				time_to_debug;
	int				time_to_refactor;
	int				nb_compiles_req;
	int				dongle_cooldown;
	int				scheduler_type;
	long long		start_time;
	int				sim_running;
	pthread_mutex_t	print_lock;
	pthread_mutex_t	state_lock;
	t_dongle		*dongles;
	t_coder			*coders;
}	t_data;

#endif
