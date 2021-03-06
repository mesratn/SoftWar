/*
** Softwar_Context.c for SoftWar in 
** 
** Made by CASTELLARNAU Aurelien
** Login   <castel_a@etna-alternance.net>
** 
** Started on  Sun Jul  9 22:32:32 2017 CASTELLARNAU Aurelien
** Last update Wed Sep 13 21:27:24 2017 BILLAUD Jean
*/

#include <stdio.h>
#include <czmq.h>
#include "libmy.h"
#include "softwar_ctx.h"

/*
** private function
*/
/*
** Fonction passée à la création de la chaine
** t_swctx->sockets
** Permet de détruire les sockets stockées
** dans une liste chainée passée en paramètre.
*/
void	free_sockets(t_chain **sockets);

/*
** global and static context.
*/
static t_swctx	*ctx = NULL;

/*
** Crée une socket du type SoftWar
** adaptée à notre contexte, c'est une
** socket avec un char * en plus pour nous...
*/
t_swsock	*create_socket(char *name, zsock_t *socket)
{
  t_swsock	*s;

  if ((s = malloc(sizeof (*s))) == NULL)
    {
      my_log(__func__, MEM_ERR, 1);
      return (NULL);
    }
  if ((s->name = my_strdup(name)) == NULL)
    return (NULL);
  s->socket = socket;
  return (s);
}

/*
** finalize_ctx
** appellée par le getter pour s'assurer de toujours
** avoir le context. Si la définition du context
** par le biais du process de parsing foire, on est sûr
** d'avoir au moins les valeurs par défaut.
** Si le ctx est bien setté de partout, finalize ne fait rien.
*/
t_swctx	*finalize_ctx()
{
  if (ctx == NULL)
    {
      if ((ctx = malloc(sizeof (*ctx))) == NULL)
	{
	  my_log(__func__, MEM_ERR, 1);
	  return (NULL);
	}
      ctx->rep_port = 0;
      ctx->pub_port = 0;
      ctx->cycle = 0;
      ctx->size = 0;
      ctx->test = 0;
      if ((ctx->sockets = create_chain(free_sockets)) == NULL)
	{
	  my_log(__func__, MEM_ERR, 1);
	  return (NULL);
	}
      if ((ctx->active_socket = malloc(sizeof ((*ctx->active_socket)))) == NULL)
	{
	  my_log(__func__, MEM_ERR, 1);
	  return (NULL);
	}
      ctx->active_socket->name = NULL;
      ctx->active_id = NULL;
    }
  if (ctx->rep_port == 0)
    ctx->rep_port = 4242;
  if (ctx->pub_port == 0)
    ctx->pub_port = 4243;
  if (ctx->cycle == 0)
    ctx->cycle = 500000;
  if (ctx->size == 0)
    ctx->size = 4;
  return (ctx);
}

/*
** Fonction conçue pour être exécutée avec les options suivantes:
** -rep-port [int >0]
** -pub-port [int > 0]
** -cycle [int > 0]
** passée comme callback au parseur d'argument, 
** cette fonction définie le SoftWar_ctx
*/
t_swctx			*init_swctx(char *opt, t_chain *parameters)
{
  t_link		*tmp;
  char			*param;

  param = NULL;
  if (parameters != NULL && parameters->dictionnary != NULL)
    {
      tmp = parameters->dictionnary[0];
      param = tmp->content;
    }
  if (ctx == NULL)
    {
      if ((ctx = malloc(sizeof (*ctx))) == NULL)
	{
	  my_log(__func__, MEM_ERR, 1);
	  return (NULL);
	}
      if ((ctx->active_socket = malloc(sizeof ((*ctx->active_socket)))) == NULL)
	{
	  my_log(__func__, MEM_ERR, 1);
	  return (NULL);
	}
      ctx->active_socket->name = NULL;
      ctx->rep_port = 0;
      ctx->pub_port = 0;
      ctx->cycle = 0;
      ctx->size = 0;
      ctx->test = 0;
      ctx->poller = NULL;
      ctx->active_id = NULL;
      if ((ctx->sockets = create_chain(free_sockets)) == NULL)
	{
	  my_log(__func__, MEM_ERR, 1);
	  return (NULL);
	}
    }
  if (!my_strcmp(opt, "-rep-port"))
    ctx->rep_port = my_getnbr(param);
  if (!my_strcmp(opt, "-pub-port"))
    ctx->pub_port = my_getnbr(param);
  if (!my_strcmp(opt, "-cycle"))
    ctx->cycle = my_getnbr(param);
  if (!my_strcmp(opt, "-map-size"))
    ctx->size = my_getnbr(param);
  if (!my_strcmp(opt, "-test"))
    ctx->test = 1;
  return (ctx);
}

t_swctx	*get_swctx()
{
  return (finalize_ctx());
}

/*
** fonction de libération de la t_chain contenue dans le ctx.
*/
void		free_sockets(t_chain **sockets)
{
  t_link	*tmp;
  t_swsock	*s;

  tmp = (*sockets)->first;
  while (tmp)
    {
      s = (t_swsock*)tmp->content;
      zsock_destroy(&(s->socket));
      free(s->name);
      tmp = tmp->next;
    }
}

/*
** Libération du ctx.
*/
void		free_ctx()
{
  if (ctx != NULL)
    {
      if (ctx->sockets != NULL)
	delete_chain(&(ctx->sockets));
      if (ctx->active_socket != NULL )
	free(ctx->active_socket->name);
      if (ctx->poller != NULL)
	zpoller_destroy(&(ctx->poller));
      if (ctx->active_id != NULL)
	free(ctx->active_id);
      free(ctx);
    }
}
