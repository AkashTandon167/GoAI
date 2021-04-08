/*
 * Game.cpp
 *
 *  Created on: Mar 31, 2021
 *      Author: akash
 */

#include "Game.h"
#include "Board.h"
#include <random>
#include <time.h>
#include <inttypes.h>
#include <cassert>
#include <iostream>
#include <sstream>
#include <algorithm>

Game::Game() : Game(MAX_BOARDSIZE) {
}

Game::Game(uint8_t board_size_) {
	goban = Board(board_size_);
	game_state = 0;
	play_num = 0;
	board_size = board_size_;
	num_vertices = (board_size + 2) * (board_size + 2);
	zobrist_table = zobristInit();
	captured_black = 0;
	captured_white = 0;
}

Game::Game(const Game &dupl) {
	goban = Board(dupl.goban);
	game_state = dupl.game_state;
	play_num = dupl.play_num;
	board_size = dupl.board_size;
	num_vertices = (board_size + 2) * (board_size + 2);
	zobrist_table = dupl.zobrist_table;
	captured_black = dupl.captured_black;
	captured_white = dupl.captured_white;
}

Game::~Game() {
}

bool Game::move(int16_t move_) {
	if (!goban.valid_vertex(move_)) { //invalid pos
		return false;
	}
	if (goban.get_state(move_) != Board::vertex_t::EMPTY) {
		return false;
	}
	if (goban.is_suicide(move_, side())) {
		return false;
	}
	uint32_t hashValue = zobristHash();
	if (koCheck(hashValue)) { //if board has occurred
		return false;
	}
	if (game_state == 2 || game_state == -2) {
		return false;
	} //game over
	if (move_ == Board::RESIGN) {
		game_state = side() ? -2 : 2; //if you resign, you lose
		return true;
	}
	if (move_ == Board::PASS) {
		if (game_state == 1 || game_state == -1) {
			game_state = (area_score(6.5) > 0) ? 2 : -2;
			return true;
		} else {
			game_state = (side()) ? 1 : -1;
			return true;
		}
	}

	goban.set_state(move_,
			(side() ? Board::vertex_t::BLACK : Board::vertex_t::WHITE));
	capture();
	play_num++;
	past_boards.insert(hashValue);
	game_state = 0;
	return true;
}

bool Game::move(uint8_t x, uint8_t y) {
	int vertex = goban.get_vertex(x, y);
	return move(vertex);
}

void Game::resign() {

}

void Game::pass() {
}

void Game::print() {
	//goban.debug();
	goban.print();
	printf("Current Score: %f\n", area_score(0));
	printf("Current Influence: %f\n", influence());
}

double Game::area_score(double komi) {
	double score = 0;
	for (int i = 0; i < num_vertices; i++) {
		if (goban.valid_vertex(i)) {
			if (goban.get_state(i) == Board::BLACK) {
				score += 1;
			} else if (goban.get_state(i) == Board::WHITE) {
				score -= 1;
			} else if (goban.is_eye(i, true)) {
				score += 2;
			} else if (goban.is_eye(i, false)) {
				score -= 2;
			}
		}
	}
	return score + komi;
}

double Game::score() {
	return area_score(0) + 0.1 * influence() + 2 * (goban.get_net_prisoners());
}

bool Game::side() {
	return (play_num % 2) == 0;
}

uint8_t Game::get_size() {
	return board_size & goban.get_boardsize();
}

bool Game::ongoing() {
	return (game_state != 2 && game_state != -2);
}

Board::vertex_t Game::get_state(uint8_t x, uint8_t y) {
	return goban.get_state(x, y);
}

/*
 std::vector<bool> Game::benson(bool side) {

 }
 */

uint8_t Game::get_neighbors(uint8_t x, uint8_t y, Board::vertex_t content) {
	return goban.get_neighbors(goban.get_vertex(x, y), content);
}

/*
 bool Game::relevant(uint8_t x, uint8_t y) {
 }
 */

int Game::get_vertex(uint8_t x, uint8_t y) const {
	return goban.get_vertex(x, y);
}

void Game::simulate(std::vector<std::string> movelist) {
	for (auto move_ : movelist) {
		if (move(goban.text_to_move(move_))) {
			print();
		} else {
			printf("INVALID MOVE!!!\n");
		}
	}
}

void Game::simulate_sgf(std::vector<std::string> movelist) {
	for (auto move_ : movelist) {
		if (move(goban.text_to_move_sgf(move_))) {
			print();
		} else {
			printf("INVALID MOVE!!!\n");
		}
	}
}

bool Game::koCheck(uint32_t hash_value) {
	return (past_boards.find(hash_value) != past_boards.end());
}

uint32_t Game::zobristHash() {
	uint32_t hashValue = 0;
	for (int i = 0; i < board_size; i++) {
		for (int j = 0; j < board_size; j++) {
			Board::vertex_t state = goban.get_state(i, j);
			if (state == Board::vertex_t::BLACK) {
				hashValue ^= zobrist_table[i * board_size + j];
			} else if (state == Board::vertex_t::WHITE) {
				hashValue ^=
						zobrist_table[i * board_size + j + board_size * board_size]; //two sets of values
			}
		}
	}
	return hashValue; /*maybe unnecessary, I could have an attribute representing the hash of
	 the current board and update that each time a new move is made
	 unless it is a capturing move */
}

bool Game::capture() {
	for (int i = 0; i < num_vertices; i++) {
		if (goban.valid_vertex(i)) {
			if (goban.get_state(i) != Board::EMPTY) {
				if (goban.get_chain_liberties(i) == 0) {
					goban.capture_chain(i); //TODO problems here
				}
			}
		}
	}
	return true;
}

std::vector<uint32_t> Game::zobristInit() {
	std::mt19937_64 randGen = std::mt19937_64(time(NULL)); //random number generator
	std::vector<uint32_t> out;
	for (int i = 0; i < board_size * board_size * 2; i++) {
		out.push_back(randGen());
	}
	return out;
}

int Game::get_prisoners() {
	return goban.get_net_prisoners();
}

double Game::influence() {
	if (play_num == 0) {
		return 0.f;
	}
	int directions[4] = { -1, -board_size, 1, board_size };
	std::vector<double> infl = std::vector<double>((board_size * board_size), 0);
	for (int i = 0; i < (board_size * board_size); i++) {
		switch (get_state(i / board_size, i % board_size)) {
			case Board::vertex_t::BLACK:
				infl[i] = 1;
				break;
			case Board::vertex_t::WHITE:
				infl[i] = -1;
				break;
			default:
				infl[i] = 0;
				break;
		}
	}

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < (board_size * board_size); j++) {
			double sum = 0;
			if (infl[j] == 0) {
				for (int d = 0; d < 4; d++) {
					if (j + directions[d] >= 0
							&& j + directions[d] < (board_size * board_size)) {
						sum += (infl[j + directions[d]]);
					}
				}
				if (sum < 2.f && sum > -2.f) {
					infl[j] += sum / 2;
				}
			}
		}
	}
	double out = 0;
	for (int i = 0; i < (board_size * board_size); i++) {
		if (infl[i] == 0.0) {
			//printf("  ");
		} else if (infl[i] == 1.0) {
			//printf(" O");
		} else if (infl[i] == -1.0) {
			//printf(" X");
		} else if (infl[i] > 0) {
			//printf(" o");
		} else if (infl[i] < 0) {
			//printf(" x");
		}
		//printf("%6.2f ", infl[i]);
		if (i % board_size == (board_size - 1)) {
			//printf("\n");
		}
		out += infl[i];
	}
	return out;
}

bool Game::is_eye(int vertex, bool side) {
	return goban.is_eye(vertex, side);
}

int Game::text_to_move(std::string move) const {
	transform(cbegin(move), cend(move), begin(move), tolower);

	if (move == "pass") {
		return Board::PASS;
	} else if (move == "resign") {
		return Board::RESIGN;
	} else if (move.size() < 2 || !std::isalpha(move[0]) || !std::isdigit(move[1])
			|| move[0] == 'i') {
		return NUM_VERTICES;
	}

	auto column = move[0] - 'a';
	if (move[0] > 'i') {
		--column;
	}

	int row;
	std::istringstream parsestream(move.substr(1));
	parsestream >> row;
	--row;

	if (row > board_size * board_size || column > board_size * board_size) {
		return NUM_VERTICES;
	}
	return get_vertex(row, column);
}

std::string Game::move_to_text(const int move) const { //standard format
	std::ostringstream result;

	int column = move % (board_size + 2) - 1;
	int row = move / (board_size + 2) - 1;

	assert(
			move == Board::PASS || move == Board::RESIGN
					|| (row >= 0 && row < board_size));
	assert(
			move == Board::PASS || move == Board::RESIGN
					|| (column >= 0 && column < board_size));

	if (move >= 0 && move < num_vertices) {
		result << static_cast<char>(column < 8 ? 'A' + column //no 'I's
																									:
																							'A' + column + 1);
		result << (row + 1);
	} else if (move == Board::PASS) {
		result << "pass";
	} else if (move == Board::RESIGN) {
		result << "resign";
	} else {
		result << "error";
	}
	return result.str();
}

bool Game::relevant(uint8_t x, uint8_t y) {
	return false;
}

int Game::get_play_num() const {
	return play_num;
}

void Game::debug() {
	goban.print_chains();
}

