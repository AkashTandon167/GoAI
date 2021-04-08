/*
 * Game.h
 *
 *  Created on: Mar 31, 2021
 *      Author: akash
 */

#ifndef GAME_H
#define GAME_H
#include "Board.h"
#include <unordered_set>
#include <functional>

class Game {
public:
	Game();
	Game(uint8_t board_size);
	Game(const Game &dupl);
	virtual ~Game();

	bool move(int16_t move_);
	bool move(uint8_t x, uint8_t y);
	void resign();
	void pass();

	void print();
	double area_score(double komi);
	double score();
	bool side();
	uint8_t get_size();
	bool ongoing();
	Board::vertex_t get_state(uint8_t x, uint8_t y);
	std::vector<bool> benson(bool side);
	uint8_t get_neighbors(uint8_t x, uint8_t y, Board::vertex_t content);
	bool relevant(uint8_t x, uint8_t y);
	int get_vertex(uint8_t x, uint8_t y) const;

	void simulate(std::vector<std::string> movelist);
	void simulate_sgf(std::vector<std::string> movelist);

	bool is_eye(int vertex, bool side);
	int text_to_move(std::string move) const;
	std::string move_to_text(const int move) const;

	int get_play_num() const;

	uint32_t zobristHash();

	int get_prisoners();

	void debug();

private:
	Board goban;
	bool koCheck(uint32_t hashValue);
	std::unordered_set<uint32_t> past_boards;
	std::vector<uint32_t> zobrist_table;
	int8_t game_state; //0 is ongoing game, +-1 is pass, +-2 is resign, black is + white is -
	uint16_t play_num;
	uint8_t board_size;
	uint16_t num_vertices;
	bool capture(); //captures all stones surrounded, returns false if self capture
	std::vector<uint32_t> zobristInit();
	double influence();
	uint16_t captured_black;
	uint16_t captured_white;
};

#endif // GAME_H
