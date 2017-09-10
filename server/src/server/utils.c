/*
** utils.c for SW in /home/naej/soft_war/Soft_war/server
** 
** Made by BILLAUD Jean
** Login   <billau_j@etna-alternance.net>
** 
** Started on  Mon Sep  4 20:05:35 2017 BILLAUD Jean
** Last update Fri Sep  8 16:56:29 2017 BILLAUD Jean
*/


#include <stdio.h>
#include <stdlib.h>
#include <json/json.h>
#include "libmy.h"
#include "Player.h"
#include "Energy_cell.h"
#include "Game_manager.h"

int		check_mvmnt(uint x, uint y, char *identity, t_game_manager *manager)
{
  char		log[50];
  t_chain       *players;
  t_chain       *ecs;
  t_map_manager *map;
  int		map_size;
  
  players = manager->get_players();
  ecs = manager->get_energy_cells();
  map = manager->map_manager();
  map_size = manager->get_map_size();
  if (map->is_wall(x, y, map_size) == 1) {
    sprintf(log, "%s tried to go in a wall", identity);
    return (1);
  } else if (map->is_free_square(x, y, players, ecs) == 1) {
    sprintf(log, "%s tried to walk on something", identity);
    return (1);
  } else
    return (0);
}