/*
** hash.c for SoftWar in 
** 
** Made by CASTELLARNAU Aurelien
** Login   <castel_a@etna-alternance.net>
** 
** Started on  Sun Jul 16 01:16:12 2017 CASTELLARNAU Aurelien
** Last update Sun Jul 16 02:00:08 2017 CASTELLARNAU Aurelien
*/

#include "libmy.h"
#include "hash.h"

/*
** La constante HCOMMANDSIZE est définie dans le .h
** Elle doit correspondre avec le nombre d'éléments
** attendus. Vilaine solution temporaire. 
*/
static char	*hash_table_command[HCOMMANDSIZE];

/*
** Algo tout simple pour créer un hash
** on ajoute le poid ascii des lettres,
** et on modulote sur le nombre total
** d'éléments souhaité.
*/
int	generate_hash(char *command)
{
  int	it;
  int	hash;

  it = 0;
  hash = 0;
  while (*(command + it))
    {
      hash += *(command + it);
      ++it;
    }
  return (hash % HCOMMANDSIZE);
}

/*
** Vérifie si une colision
** a été ouverte pour cette command
** de cette manière on itère que
** sur les quelques colisions.
*/
int		is_colision(t_chain *colisions, char *command)
{
  t_link	*tmp;
  t_colision	*colision;
  char		log[60];

  tmp = colisions->first;
  while (tmp)
    {
      colision = (t_colision*)tmp->content;
      if (!my_strcmp(colision->command, command))
	{
	  sprintf(log, "%s is colision, hash retrieved", command);
	  my_log(__func__, log, 3);
	  return (colision->hash);
	}
      tmp = tmp->next;
    }
  sprintf(log, "%s isnt a colision", command);
  my_log(__func__, log, 3);
  return (NOT_COLISION);
}

/*
** Vérifie si l'on a trouvé le bon hash
*/
int	is_good_hash(int hash, char *command)
{
  return (hash_table_command[hash] != NULL
	  && !my_strcmp(hash_table_command[hash], command));
}

/*
** On cherche un hash pas pris et
** on ouvre une colision pour sauvegarder
** la correspondance command/hash.
** c'est sur cette liste qu'on itère dans le pire
** des cas (si le bon hash n'a pas tout de suite
** été matché). Limitant les itérations aux quelques
** commandes ou generate_hash produit le même hash.  
*/
int		resolve_colision(int hash, char *command, t_chain **colisions)
{
  t_colision	*new_colision;
  int		limit;
  char		log[60];
  
  limit = 0;
  sprintf(log, "resolve colision start hash value: %d", hash);
  my_log(__func__, log, 4);
  while (hash_table_command[hash] != NULL && limit < HCOMMANDSIZE)
    {
      if (hash == (HCOMMANDSIZE - 1))	   // On déplace le hash entre 0 et max.
	hash = 0;
      else
	++hash;
      ++limit;
    }
  sprintf(log, "hash determined: %d", hash);
  my_log(__func__, log, 4);
  if (limit == HCOMMANDSIZE) // Si on a fait le tour, c'est que toute les commandes sont prises
    {
      sprintf(log, "%s command unknow", command);
      my_log(__func__, log, 3);
      return (HCOMMANDUNKNOW); // on est dans le cas d'un input qui n'est pas une commande
    }
  if ((new_colision = malloc(sizeof(*new_colision))) == NULL) // On ajoute la correspondance dans la liste des colisions
    {
      my_log(__func__, MEM_ERR, 1);
      return (HCOMMANDERR);
    }
  new_colision->hash = hash;
  if ((new_colision->command = my_strdup(command)) == NULL)
    return (HCOMMANDERR);
  if (add_link(colisions, new_colision))
    return (HCOMMANDERR);
  sprintf(log, "hash out of resolve colision: %d", hash);
  my_log(__func__, log, 3);
  return (hash);
}

/*
** free du char* dans colision
*/
void		free_colisions(t_chain **colisions)
{
  t_link	*tmp;
  t_colision	*c;

  if (chain_is_null(colisions)
      || chain_is_empty(colisions))
    return;
  tmp = (*colisions)->first;
  while (tmp)
    {
      c = (t_colision*)tmp->content;
      if (c != NULL)
	{
	  if (c->command != NULL)
	    free(c->command);
	  free(c);
	}
      tmp = tmp->next;
    }
}

/*
** les colisions enregistrées au premier
** appel de src/server/command.c::get_commands()
** le sont pour toute la durée du runtime.
*/
t_chain			*init_colisions()
{
  static t_chain	*colisions = NULL;

  if (colisions == NULL && (colisions = create_chain(free_colisions)) == NULL)
    return (NULL);
  return (colisions);
}

/*
** process de hashage des noms des commandes
** permet un accès direct aux commandes par le
** nom de celle ci en effectuant un maximum d'accès
** direct aux index et en diminuant le nombre d'itérations.
** Les commandes sont appellées une par une au travers
** de la fonction hash_command lorsqu'on les get la première fois
** (voir /src/server/command.c).
** Au second appel, soit on retrouve la commande avec is_colision,
** soit avec generate_hash. Soit avec quelques itérations, soit avec
** une simple boucle qui additionne. Un peu overkill pour Softwar
** mais le principe peut s'étendre et c'était sympas à coder ;).
*/
int		hash_command(char *c)
{
  int		hash;
  char		log[50];
  char		*command;
  t_chain	*colisions;
  
  hash = 0;
  command = c;
  sprintf(log, "command in hash process: %s", command);
  my_log(__func__, log, 4);
  if ((colisions = init_colisions()) == NULL) // On récupère le singleton des colisions.
    return (HCOMMANDERR);
  /*
  ** On vérifie si le hash est une colision, 
  ** si c'est le cas, on récupère d'office le bon hash!
  */
  if ((hash = is_colision(colisions, command)) == NOT_COLISION) 
    {
      /*
      ** si ce n'est pas une colision, on génère le hash
      */
      hash = generate_hash(command);
      /*
      ** si le hash n'est pas le bon on a: 
      ** un hash qui n'est pas une colision 
      ** et qui n'est pas bon en direct
      ** cela signifie que le hash n'existe pas encore. 
      */
      if (!is_good_hash(hash, command))
	{
	  my_log(__func__, "generate new hash or command unknow", 3);
	  /*
	  ** si le hash est déjà pris
	  */
	  if (hash_table_command[hash] != NULL
	      /*
	      ** on a un hash qui n'est pas encore enregitré 
	      ** mais qui colisione, donc on resoud la colision.
	      */
	      && (hash = resolve_colision(hash, command, &colisions)) == HCOMMANDERR)
	    return (HCOMMANDERR);
	  /*
	  ** si la colision n'est pas résolue, 
	  ** on est dans le cas d'une commande inexistante 
	  ** et d'un tableau de commandes plein.
	  */
	  if (hash == HCOMMANDUNKNOW)
	    {
	      my_log(__func__, "command unknow", 3);
	      return (HCOMMANDUNKNOW);
	    }
	  /*
	  ** On enregistre la correspondance dans la table de hash
	  */
	  if ((hash_table_command[hash] = my_strdup(command)) == NULL)
	    return (HCOMMANDERR);
	  sprintf(log, "new hash generate: %d", hash);
	  my_log(__func__, log, 3);
	  return (hash);
	}
    }
  else
    /*
    ** là c'était une colision, on récupère le hash directement.
    */
    my_log(__func__, "is colision, hash retrieved by the small collision table!!", 4);
  /*
  ** Ce test est un peu inutile, tout les cas sont
  ** déjà traités (normalement...). Sécu supplémentaire...
  */
  if (is_good_hash(hash, c))
    {
      sprintf(log, "hash for command %s successfully retrieve, value = %d", c, hash);
      my_log(__func__, log, 4);
      return (hash);
    }
  return (HCOMMANDERR);
}

void		free_hashes()
{
  int		it;
  t_chain	*colisions;
  
  colisions = init_colisions();
  delete_chain(&colisions);
  for (it = 0; it < HCOMMANDSIZE; ++it)
    {
      if (hash_table_command[it] != NULL)
	free(hash_table_command[it]);
    }
}
