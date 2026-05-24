/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bbeaurai <bbeaurai@student.42lehavre.fr    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/21 16:01:27 by bbeaurai          #+#    #+#             */
/*   Updated: 2026/05/24 10:25:03 by bbeaurai         ###   ########.fr       */
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
}					t_dongle;

typedef struct s_coder
{
	int				id;
	pthread_t		thread_id;
	int				nb_compiles;
	long long		last_compile_start;
	t_dongle		*left_dongle;
	t_dongle		*right_dongle;
	struct s_data	*data;
}					t_coder;

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
	pthread_mutex_t	sched_lock;
	pthread_cond_t	sched_cond;
	int				*wait_queue;
	int				wait_count;
	t_dongle		*dongles;
	t_coder			*coders;
}					t_data;

int			validate_args(int argc, char **argv);
int			init_system(t_data *data);
void		cleanup_system(t_data *data);

int			init_threads(t_data *data, pthread_t *monitor);
void		wait_threads(t_data *data, pthread_t monitor);

void		sched_enqueue(t_data *data, int coder_id);
void		sched_dequeue(t_data *data, int coder_id);

void		scheduler_request(t_data *data, t_coder *coder);
void		scheduler_release(t_data *data, t_coder *coder);

void		*coder_routine(void *arg);
void		*monitor_thread(void *arg);

void		print_state(t_data *data, int id, const char *msg);

int			ft_atoi(const char *s);
int			is_number(const char *s);
long long	current_time_ms(void);

#endif
