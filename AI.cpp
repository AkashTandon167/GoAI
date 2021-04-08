/*
 * AI.cpp
 *
 *  Created on: Mar 31, 2021
 *      Author: akash
 */

#include "Game.h"
#include "Board.h"
#include <random>
#include <thread>
#include <chrono>
#include <iostream>
#include <windows.h>
#include "AI.h"

double minScore = -std::numeric_limits<double>::max();
double maxScore = std::numeric_limits<double>::max();

double vertex_influence(int x, int y, uint8_t board_size) {
	x = (x > 9) ? (board_size - x) : x;
	y = (y > 9) ? (board_size - y) : y;

	double x_factor = 1 / (((double) x - 3) * ((double) x - 3) + 1) + 0.5;
	double y_factor = 1 / (((double) y - 3) * ((double) x - 3) + 1) + 0.5;
	return x_factor * y_factor;
}

double minimax(Game input, uint8_t depth, double alpha, double beta) {
	if (depth <= 0) {
		return input.score();
	}
	int size = input.get_size();
	int x_offset = rand() % size;
	int y_offset = rand() % size;
	double bestScore = input.side() ? minScore : maxScore;
	if (input.side()) {
		for (int i = 0; i < size; i++) {
			for (int j = 0; j < size; j++) {
				int move_x = (i + x_offset) % size;
				int move_y = (j + y_offset) % size;
				Game test = Game(input);
				if (test.move(move_x, move_y)) {
					double score = minimax(test, depth - 1, alpha, beta);
					if (score > bestScore) {
						bestScore = score;
					}

					if (bestScore > alpha) {
						alpha = bestScore;
					}

					if (alpha >= beta) {
						return bestScore;
					}
				}
			}
		}
	} else {
		for (int i = 0; i < size; i++) {
			for (int j = 0; j < size; j++) {
				int move_x = (i + x_offset) % size;
				int move_y = (j + y_offset) % size;
				Game test = Game(input);
				if (test.move(move_x, move_y)) {
					double score = minimax(test, depth - 1, alpha, beta);
					if (score < bestScore) {
						bestScore = score;
					}

					if (bestScore < beta) {
						beta = bestScore;
					}

					if (alpha >= beta) {
						return bestScore;
					}
				}
			}
		}
	}

	return bestScore;
}

int fuseki(Game input) {
	int size = input.get_size();
	int move_seq_0[] = { input.get_vertex(3, size - 4), input.get_vertex(size - 4,
			3), input.get_vertex(size - 4, size - 4), input.get_vertex(3, 3),
			input.get_vertex(size - 3, 5), input.get_vertex(size - 6, 2),
			input.get_vertex(2, 2) };
	Game x = Game(input.get_size());
	for (int move : move_seq_0) {
		if (input.zobristHash() == x.zobristHash()) {
			return move;
		}
		x.move(move);
	}
	return 0;
}

int bestMove(Game input, uint8_t depth) {
	int size = input.get_size();
	int x_offset = rand() % size;
	int y_offset = rand() % size;
	int bestX = rand() % size;
	int bestY = rand() % size;
	double bestScore = (input.side()) ? minScore : maxScore;
	if (input.side()) {
		for (int i = 0; i < size; i++) {
			for (int j = 0; j < size; j++) {
				int move_x = (i + x_offset) % size;
				int move_y = (j + y_offset) % size;
				Game test = Game(input);
				if (test.move(move_x, move_y)) {
					double score = minimax(test, depth - 1, minScore, maxScore);
					if (score > bestScore) {
						bestScore = score;
						bestX = move_x;
						bestY = move_y;
					}
				}
			}
		}
	} else {
		for (int i = 0; i < size; i++) {
			for (int j = 0; j < size; j++) {
				int move_x = (i + x_offset) % size;
				int move_y = (j + y_offset) % size;
				Game test = Game(input);
				if (test.move(move_x, move_y)) {
					double score = minimax(test, depth - 1, minScore, maxScore);
					if (score < bestScore) {
						bestScore = score;
						bestX = move_x;
						bestY = move_y;
					}
				}
			}
		}
	}
	return input.get_vertex(bestX, bestY);
}

