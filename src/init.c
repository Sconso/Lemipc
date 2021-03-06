/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sconso <sconso@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2014/06/01 22:46:28 by sconso            #+#    #+#             */
/*   Updated: 2014/06/01 22:52:11 by sconso           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <lemipc.h>
#include <ft_fc_conversion.h>
#include <ft_fc_print.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <stdlib.h>

static void		init_map(t_game *game)
{
	int			i;
	int			j;

	i = -1;
	while (++i < game->height)
	{
		j = -1;
		while (++j < game->width)
			game->map[i][j] = -1;
	}
}

static t_info	*init_info(char *team, t_game *game)
{
	t_info		*info;

	if (!(info = (t_info *)malloc(sizeof(t_info))))
		ft_exit("Can't malloc the info\n");
	info->team = ft_atoi(team);
	info->game = game;
	return (info);
}

static t_game	*shm_init(char **av, int *shmid)
{
	t_game		*game;
	key_t		key;

	key = get_key(av[0]);
	if ((*shmid = shmget(key, sizeof(t_game), SHM_R | SHM_W)) < 0)
	{
		*shmid = shmget(key, sizeof(t_game), IPC_CREAT | SHM_R | SHM_W);
		ft_assert(*shmid >= 0, "Can't shmget\n");
		if ((game = (t_game *)shmat(*shmid, NULL, 0)) == (t_game *)-1)
			ft_exit("Can't shmat\n");
		game->width = WIDTH;
		game->height = HEIGHT;
		game->players = 1;
		init_map(game);
		ft_putstr("Initialized the game !\n");
	}
	else
	{
		if ((game = (t_game *)shmat(*shmid, NULL, 0)) == (t_game *)-1)
			ft_exit("Can't shmat\n");
		game->players++;
		ft_putstr("Hello !\n");
	}
	return (game);
}

static int		sem_init(char **av)
{
	int			semid;
	key_t		key;

	key = get_key(av[0]);
	if ((semid = semget(key, 1, IPC_R | IPC_W)) < 0)
	{
		if ((semid = semget(key, 1, IPC_CREAT | IPC_R | IPC_W)) == -1)
			ft_exit("Can't semget\n");
		if (semctl(semid, 0, SETVAL, 1) == -1)
			ft_exit("Can't semctl\n");
	}
	return (semid);
}

t_info			*init(int ac, char **av)
{
	t_game		*game;
	t_info		*info;
	int			shmid;
	int			semid;

	(void)ac;
	game = shm_init(av, &shmid);
	semid = sem_init(av);
	info = init_info(av[1], game);
	info->shmid = shmid;
	info->semid = semid;
	info->msgqid = msgq_init(info, av[0]);
	if (info->game->players >= info->game->width * info->game->height)
		shmclear(&info, 4);
	place(info);
	get_info(info);
	return (info);
}
