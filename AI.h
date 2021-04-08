/*
 * AI.h
 *
 *  Created on: Mar 31, 2021
 *      Author: akash
 */

#ifndef AI_H_
#define AI_H_

#include "Game.h"
#include "Board.h"
#include <random>
#include <thread>
#include <chrono>
#include <iostream>

double vertex_influence(int x, int y, uint8_t board_size);

double minimax(Game input, uint8_t depth, double alpha, double beta);

int fuseki(Game input);

int bestMove(Game input, uint8_t depth);

#endif /* AI_H_ */
