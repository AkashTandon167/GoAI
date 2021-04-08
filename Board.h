/*
 * Board.h
 *
 *  Created on: Mar 31, 2021
 *      Author: akash
 */

#ifndef BOARD_H_
#define BOARD_H_
#include <cstdint>
#include <vector>
#include <array>
#include <cassert>
#define MAX_BOARDSIZE 19
#define NUM_VERTICES (MAX_BOARDSIZE + 2)*(MAX_BOARDSIZE + 2)

class Board {
public:
	Board();
	Board(uint8_t size);
	enum vertex_t : uint8_t {
		EMPTY = 0, BLACK = 1, WHITE = 2, INVAL = 3
	};

	static constexpr int PASS = -1; //vertex of pass
	static constexpr int RESIGN = -2; //vertex of resign

	uint8_t get_boardsize() const;

	double area_score(double komi) const;

	std::string move_to_text(const int16_t move) const;
	int text_to_move(std::string move) const;
	std::string move_to_text_sgf(int16_t move) const;
	int text_to_move_sgf(std::string move) const;

	uint8_t liberties(uint16_t vertex) const;
	uint8_t liberties(uint8_t x, uint8_t y) const;

	int directions[4]; // movement directions 4 way

	vertex_t get_state(uint8_t x, uint8_t y) const;
	vertex_t get_state(uint16_t vertex) const;
	void set_state(uint8_t x, uint8_t y, vertex_t content);
	void set_state(uint16_t vertex, vertex_t content);
	int get_vertex(uint8_t x, uint8_t y) const;
	std::pair<uint8_t, uint8_t> get_xy(uint16_t vertex) const;

	void print() const;

	bool valid_vertex(uint16_t vertex) const; //not in a INVAL location at edge of board

	int get_neighbors(uint16_t vertex, vertex_t value) const;

	struct Chain {
		bool side;
		std::vector<uint8_t> vertices;
		int num_stones;
		int num_liberties;
	};

	bool add_stone(uint8_t chain_index, uint16_t vertex);
	bool remove_liberty(uint8_t chain_index, uint16_t vertex);
	bool add_liberty(uint8_t chain_index, uint16_t vertex);
	void delete_chain(uint8_t chain_index);
	void capture_chain(uint16_t vertex);
	bool remove_stone(uint16_t vertex);

	bool check_chains() const;

	void print_chain(uint8_t chain_index) const;

	bool is_suicide(uint16_t vertex, bool side) const;
	bool is_eye(uint16_t vertex, bool side) const;

	int get_net_prisoners() const;

	int get_chain_liberties(uint16_t vertex) const;

	void print_chains() const;

	virtual ~Board();
protected:

	uint8_t board_size;
	uint16_t num_vertices;

	std::vector<vertex_t> board;
	std::vector<Chain> chains; //a chain in every position in the array
	//if one is edited then all others will because of reference types
	std::vector<uint8_t> chain_reps; //index of chain in chains

	bool is_starpoint(uint16_t vertex) const;
	std::array<uint16_t, 2> num_prisoners; //black, white

	void merge(uint16_t chain1, uint16_t chain2);

};

#endif /* BOARD_H_ */
