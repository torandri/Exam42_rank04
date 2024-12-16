/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   microshell.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: torandri <torandri@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/12 10:13:57 by torandri          #+#    #+#             */
/*   Updated: 2024/12/16 18:49:16 by torandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <stdio.h>

void	ft_putstr_fd(char *str, int fd)
{
	int	i;

	i = 0;
	while (str[i])
		write(fd, &str[i++], 1);
}

int	count_pipe(char **argv)
{
	int	i;
	int	nbr_pipe;

	i = 0;
	nbr_pipe = 0;
	while (argv[i] && strcmp(argv[i], ";"))
	{
		if (!strcmp(argv[i], "|"))
			nbr_pipe++;
		i++;
	}
	return (nbr_pipe);
}

char	*ft_strdup(char *str)
{
	int		i;
	char	*dupped;

	if (!str)
		return (NULL);
	i = 0;
	while (str[i])
		i++;
	dupped = (char *)malloc(sizeof(char) * (i + 1));
	if (!dupped)
		return (NULL);
	i = -1;
	while (str[++i])
		dupped[i] = str[i];
	dupped[i] = '\0';
	return (dupped);
}

void	ft_free(char **array)
{
	int	i;

	i = 0;
	while (array[i])
		free(array[i++]);
	free(array);
}

void	error_fatal(char **cmd_arg)
{
	ft_putstr_fd("error: fatal\n", 2);
	free(cmd_arg);
	exit(EXIT_FAILURE);
}

char	**extract_arg(char **argv, int *index)
{
	int		i;
	int		tmp;
	char	**cmd_arg;

	i = 0;
	tmp = *index;
	while (argv[*index] && \
	strcmp(argv[*index], ";") && strcmp(argv[*index], "|"))
	{
		(*index)++;
		i++;
	}
	cmd_arg = (char **)malloc(sizeof(char *) * (i + 1));
	if (!cmd_arg)
		return (NULL);
	*index = tmp;
	i = 0;
	while (argv[*index] && \
	strcmp(argv[*index], ";") && strcmp(argv[*index], "|"))
		cmd_arg[i++] = ft_strdup(argv[(*index)++]);
	cmd_arg[i] = NULL;
	while (argv[*index] && \
	(!strcmp(argv[*index], ";") || !strcmp(argv[*index], "|")))
		(*index)++;
	return (cmd_arg);
}

void	builtin_cd(char **cmd_arg)
{
	int	i;

	i = 0;
	while (cmd_arg[i])
		i++;
	if (i != 2)
		ft_putstr_fd("error: cd: bad arguments\n", 2);
	else if (chdir(cmd_arg[1]))
	{
		ft_putstr_fd("error: cd: cannot change directory to ", 2);
		ft_putstr_fd(cmd_arg[1], 2);
		ft_putstr_fd("\n", 2);
	}
	ft_free(cmd_arg);
}

void	execute_cmd(char **cmd_arg, char **envp, int i, int nbr_pipe)
{
	int			pid;
	int			pipefd[2];
	static int	fd_in = 0;

	if (i == 0)
		fd_in = 0;
	if (i < nbr_pipe)
	{
		if (pipe(pipefd))
			error_fatal(cmd_arg);
	}
	pid = fork();
	if (pid == -1)
		error_fatal(cmd_arg);
	else if (pid == 0)
	{
		if (dup2(fd_in, STDIN_FILENO) == -1)
			error_fatal(cmd_arg);
		if (i < nbr_pipe)
		{
			if (dup2(pipefd[1], STDOUT_FILENO) == -1)
			{
				close(fd_in);
				error_fatal(cmd_arg);
			}
			close(pipefd[0]);
			close(pipefd[1]);
		}
		execve(cmd_arg[0], cmd_arg, envp);
		ft_putstr_fd("error: cannot execute ", 2);
		ft_putstr_fd(cmd_arg[0], 2);
		ft_putstr_fd("\n", 2);
		ft_free(cmd_arg);
		exit(EXIT_FAILURE);
	}
	else
	{
		if (fd_in != 0)
			close(fd_in);
		if (i < nbr_pipe)
		{
			close(pipefd[1]);
			fd_in = pipefd[0];
		}
		if (cmd_arg)
			ft_free(cmd_arg);
		waitpid(pid, NULL, 0);
	}
}

int	main(int argc, char **argv, char **envp)
{
	int		i;
	int		index;
	int		nbr_pipe;
	char	**cmd_arg;

	if (argc == 1)
		return (0);
	index = 1;
	while (argv[index])
	{
		i = -1;
		nbr_pipe = count_pipe(&argv[index]);
		while (++i <= nbr_pipe)
		{
			cmd_arg = extract_arg(argv, &index);
			if (cmd_arg && cmd_arg[0])
			{
				if (!strcmp(cmd_arg[0], "cd"))
					builtin_cd(cmd_arg);
				else
					execute_cmd(cmd_arg, envp, i, nbr_pipe);
			}
		}
	}
	return (0);
}
