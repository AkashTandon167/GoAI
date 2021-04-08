/*
 * Board.cpp
 *
 *  Created on: Mar 31, 2021
 *      Author: akash
 */

#include "Board.h"
#include <cassert>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <vector>
#include <chrono>

Board::Board() : Board(MAX_BOARDSIZE) {

}

Board::Board(uint8_t size) {
	board_size = size;
	num_vertices = (board_size + 2) * (board_size + 2); //(19 + 2)*(19 + 2)
	num_prisoners[0] = 0;
	num_prisoners[1] = 0;
	directions[0] = -1;
	directions[1] = -board_size - 2;
	directions[2] = 1;
	directions[3] = board_size + 2;

	chains = std::vector<Chain>();
	board = std::vector<vertex_t>(num_vertices, Board::vertex_t::EMPTY);
	chain_reps = std::vector<uint8_t>(num_vertices, 255);

	for (int i = 0; i < board_size + 2; i++) {
		board[i] = Board::vertex_t::INVAL;
		board[i * (board_size + 2)] = Board::vertex_t::INVAL;
		board[i * (board_size + 2) + board_size + 1] = Board::vertex_t::INVAL;
		board[i + (board_size + 2) * (board_size + 1)] = Board::vertex_t::INVAL;
	}
}

uint8_t Board::get_boardsize() const {
	return board_size;
}

std::string Board::move_to_text(const int16_t move) const { //standard format
	assert(valid_vertex(move));

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

int Board::text_to_move(std::string move) const {

	transform(cbegin(move), cend(move), begin(move), tolower);

	if (move == "pass") {
		return PASS;
	} else if (move == "resign") {
		return RESIGN;
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

std::string Board::move_to_text_sgf(const int16_t move) const {

	assert(valid_vertex(move));

	std::ostringstream result;

	int column = move % (board_size + 2) - 1;
	int row = move / (board_size + 2) - 1;

	assert(
			move == Board::PASS || move == Board::RESIGN
					|| (row >= 0 && row < board_size));
	assert(
			move == Board::PASS || move == Board::RESIGN
					|| (column >= 0 && column < board_size));

// SGF inverts rows
	row = board_size - row - 1;

	if (move >= 0 && move < NUM_VERTICES) {
		if (column <= 25) {
			result << static_cast<char>('a' + column);
		} else {
			result << static_cast<char>('A' + column - 26);
		}
		if (row <= 25) {
			result << static_cast<char>('a' + row);
		} else {
			result << static_cast<char>('A' + row - 26);
		}
	} else if (move == Board::PASS) {
		result << "tt";
	} else if (move == Board::RESIGN) {
		result << "tt";
	} else {
		result << "error";
	}

	return result.str();
}

int Board::text_to_move_sgf(std::string move) const {

	transform(cbegin(move), cend(move), begin(move), tolower);

	if (move == "pass") {
		return PASS;
	} else if (move == "resign") {
		return RESIGN;
	} else if (move.size() < 2 || !std::isalpha(move[0])
			|| !std::isalpha(move[1])) {
		return NUM_VERTICES;
	}

	uint8_t column = move[0] - 'a';
	uint8_t row = move[1] - 'a';

	if (row > board_size * board_size || column > board_size * board_size) {
		return NUM_VERTICES;
	}
	return get_vertex(row, column);
}

uint8_t Board::liberties(uint16_t vertex) const {
	assert(valid_vertex(vertex));
	return get_neighbors(vertex, Board::EMPTY);

}

uint8_t Board::liberties(uint8_t x, uint8_t y) const {
	assert(x >= 0 && y >= 0);
	assert(x < board_size && y < board_size);
	return get_neighbors(get_vertex(x, y), Board::EMPTY);
}

Board::vertex_t Board::get_state(uint8_t x, uint8_t y) const {
	assert(x >= 0 && y >= 0);
	assert(x < board_size && y < board_size);
	return board[get_vertex(x, y)];
}

Board::vertex_t Board::get_state(uint16_t vertex) const {
	assert(valid_vertex(vertex));
	return board[vertex];
}

void Board::set_state(uint8_t x, uint8_t y, vertex_t content) {
	assert(x >= 0 && y >= 0);
	assert(x < board_size && y < board_size);
	assert(get_state(x, y) != Board::vertex_t::INVAL); //cannot change inval
	assert(content != Board::vertex_t::INVAL); //inval is board edges
	int vertex = get_vertex(x, y);
	assert(valid_vertex(vertex));
	set_state(vertex, content);
}

void Board::set_state(uint16_t vertex, vertex_t new_state) {
	assert(valid_vertex(vertex));
	assert(board[vertex] != Board::vertex_t::INVAL); //cannot change inval
	assert(new_state != Board::vertex_t::INVAL); //cannot change to inval, inval is board edges
	vertex_t previous_state = board[vertex];
	//assert(previous_state != new_state); //cant change to the same state
	board[vertex] = new_state;

	assert(previous_state == EMPTY && new_state != EMPTY);
	for (int i = 0; i < 4; i++) {
		if (valid_vertex(vertex + directions[i])) {
			if (chain_reps[vertex + directions[i]] != 255) {
				remove_liberty(chain_reps[vertex + directions[i]], vertex);
			}
		}
	}

	if (get_neighbors(vertex, new_state) == 0) {
		Chain x;
		x.side = (new_state == BLACK);
		x.vertices = std::vector<uint8_t>(num_vertices, 0);
		x.num_liberties = 0;
		x.num_stones = 0;
		chains.push_back(x);
		chain_reps[vertex] = chains.size() - 1;
		add_stone(chain_reps[vertex], vertex);
	} else if (get_neighbors(vertex, new_state) == 1) {
		for (int i = 0; i < 4; i++) { //finds neighbor and adds stone to chain
			int neighbor = vertex + directions[i];
			if (valid_vertex(neighbor)) {
				if (get_state(neighbor) == new_state) {
					if (chain_reps[neighbor] != 255) {
						add_stone(chain_reps[neighbor], vertex);
						chain_reps[vertex] = chain_reps[neighbor];
					}

				}
			}
		}
	} else {
		for (int i = 0; i < 4; i++) {
			int neighbor = vertex + directions[i];
			if (valid_vertex(neighbor)) {
				if (get_state(neighbor) == new_state) {
					add_stone(chain_reps[neighbor], vertex);
					chain_reps[vertex] = chain_reps[neighbor];
				}
			}
		}
		for (int i = 0; i < 4; i++) {
			int neighbor = vertex + directions[i];
			if (valid_vertex(neighbor)) {
				if (get_state(neighbor) == new_state) {
					merge(vertex, neighbor); //TODO fix bug
				}
			}
		}
	}
}

int Board::get_vertex(uint8_t x, uint8_t y) const {
	assert(x >= 0 && y >= 0);
	assert(x < board_size && y < board_size);
	return ((x + 1) * (board_size + 2) + (y + 1));
}

std::pair<uint8_t, uint8_t> Board::get_xy(uint16_t vertex) const {
	assert(valid_vertex(vertex));
	return std::make_pair(vertex / (board_size + 2) - 1,
			vertex % (board_size + 2) - 1);
}

void Board::print() const {
	for (uint16_t i = 0; i < num_vertices; i++) {
		switch (board[i]) {
			case vertex_t::BLACK:
				printf("O ");
				break;
			case vertex_t::WHITE:
				printf("X ");
				break;
			case vertex_t::INVAL:
				printf("# ");
				break;
			case vertex_t::EMPTY:
				if (is_eye(i, true) || is_eye(i, false)) {
					printf(". ");
				} else if (is_starpoint(i)) {
					printf("* ");
				} else {
					printf("  ");
				}

		}
		if ((i + 1) % (board_size + 2) == 0) {
			printf("\n");
		}
	}
//	print_chains();
}

bool Board::valid_vertex(uint16_t vertex) const {
	if (vertex < (board_size + 2)) {
		return false;
	}
	if (vertex % (board_size + 2) == 0) {
		return false;
	}
	if ((vertex + 1) % (board_size + 2) == 0) {
		return false;
	}
	if (vertex > (board_size + 2) * (board_size + 1)) {
		return false;
	}
	return true;
}

int Board::get_neighbors(uint16_t vertex, vertex_t value) const {
	assert(valid_vertex(vertex));
	int count = 0;
	for (int i = 0; i < 4; i++) {
		if (board[vertex + directions[i]] == value) {
			count++;
		}
	}
	return count;
}

Board::~Board() {
// TODO Auto-generated destructor stub
}

double Board::area_score(double komi) const {
	return 0;
}

bool Board::is_starpoint(uint16_t vertex) const {
	//TODO complete this method
	assert(valid_vertex(vertex));
	if (board_size == 19) {
		std::pair<int, int> coords = get_xy(vertex);
		int x = coords.first;
		int y = coords.second;
		if (x == 3 || x == 9 || x == 15) {
			if (y == 3 || y == 9 || y == 15) {
				return true;
			}
		}
	}
	return false;

}

bool Board::check_chains() const {
	for (int i = 0; i < num_vertices; i++) {
		if (valid_vertex(i)) {
			bool value = (chains[0].vertices[i] == 1);
			for (Chain c : chains) {
				if (c.vertices[i] == 1 && value) { //two chains cannot both have a stone
					return false;
				} else {
					value = value || (c.vertices[i] == 1);
				}
			}
		}
	}
	return true;
}

void Board::print_chain(uint8_t chain_index) const {

}

void Board::print_chains() const {
	for (uint16_t i = 0; i < num_vertices; i++) {
		if (chain_reps[i] == 255) {
			printf("    ");
		} else {
			printf("%3d ", chain_reps[i]);
		}
		if ((i + 1) % (board_size + 2) == 0) {
			printf("\n");
		}
	}

	for (uint16_t i = 0; i < num_vertices; i++) {
		if (valid_vertex(i)) {
			if (chain_reps[i] < chains.size()) {
				printf("%3d ", chains[chain_reps[i]].num_liberties);
			} else {
				printf("    ");
			}
		}

		if ((i + 1) % (board_size + 2) == 0) {
			printf("\n");
		}
	}
	printf("%u\n", chains.size());
}

void Board::merge(uint16_t chain1, uint16_t chain2) {
	assert(valid_vertex(chain1) && valid_vertex(chain2));
//TODO test method
	if (chain1 == chain2) {
		return; //same intersection
	}
	if (chain_reps[chain1] == chain_reps[chain2]) {
		return; //same chain
	}
	if (chain_reps[chain1] == 255 || chain_reps[chain2] == 255) {
		return;
	}
	Chain current = chains[chain_reps[chain1]];
	Chain other = chains[chain_reps[chain2]];
	if (current.side != other.side) {
		return;
	}
	Chain new_chain;
	new_chain.side = current.side;
	new_chain.vertices = std::vector<uint8_t>(num_vertices, 0);
	new_chain.num_liberties = 0;
	new_chain.num_stones = 0;
	for (int i = 0; i < num_vertices; i++) {
		if (valid_vertex(i)) {
			if ((current.vertices[i] == 1) || (other.vertices[i] == 1)) {
				new_chain.vertices[i] = 1;
				new_chain.num_stones++;
			} else if ((current.vertices[i] == 2) || (other.vertices[i] == 2)) {
				new_chain.vertices[i] = 2;
				new_chain.num_liberties++;
			}
		}
	}
	delete_chain(chain_reps[chain1]);
	delete_chain(chain_reps[chain2]);
	for (int i = 0; i < num_vertices; i++) { //do this after deleting subchains, because deleting them resets chain_reps
		if (valid_vertex(i)) {
			if (new_chain.vertices[i] == 1) {
				chain_reps[i] = chains.size();
			}
		}
	}
	chains.push_back(new_chain);
}

bool Board::add_stone(uint8_t chain_index, uint16_t vertex) {
	assert(valid_vertex(vertex));
	assert(chain_index < chains.size());
	if (chains[chain_index].vertices.at(vertex) == 1) { //already stone
		return false;
	} else if (board[vertex] != ((chains[chain_index].side) ? BLACK : WHITE)) { //we change board before chains
		return false;
	} else if (chains[chain_index].vertices.at(vertex) == 2) {
		remove_liberty(chain_index, vertex); //possible bug
	}
	chains[chain_index].num_stones++;
	chains[chain_index].vertices.at(vertex) = 1;
	chain_reps[vertex] = chain_index;
	for (int i = 0; i < 4; i++) {
		if (valid_vertex(vertex + directions[i])) {
			if (chains[chain_index].vertices.at(vertex + directions[i]) == 0) {
				add_liberty(chain_index, vertex + directions[i]);
			}
		}
	}
	return true;
}

bool Board::add_liberty(uint8_t chain_index, uint16_t vertex) {
	assert(valid_vertex(vertex));
	assert(chain_index < chains.size());
	if (board[vertex] != EMPTY) {
		return false;
	}
	if (chains[chain_index].vertices.at(vertex) != 0) { //can't be already a stone or liberty
		return false;
	}
	chains[chain_index].vertices.at(vertex) = 2;
	chains[chain_index].num_liberties++;
	return false;
}

bool Board::remove_stone(uint16_t vertex) {
	assert(valid_vertex(vertex));
	if (board[vertex] == EMPTY) {
		return false;
	}
	board[vertex] = EMPTY;
	chains[chain_reps[vertex]].vertices[vertex] = 0;
	chains[chain_reps[vertex]].num_stones--;
	chain_reps[vertex] = 255;
	for (int i = 0; i < 4; i++) {
		if (valid_vertex(vertex + directions[i])) {
			if (chain_reps[vertex + directions[i]] < chains.size()) { //we can't remove liberties unless theres a chain
				remove_liberty(chain_reps[vertex + directions[i]], vertex); //possible bug
			}
		}
	}
	return true;
}

bool Board::remove_liberty(uint8_t chain_index, uint16_t vertex) {
	assert(valid_vertex(vertex));
	assert(chain_index < chains.size());
	if (board[vertex] == EMPTY) { //we change board before chains
		return false;
	}
	if (chains[chain_index].vertices.at(vertex) != 2) { //can't be already a stone or liberty
		return false;
	}
	chains[chain_index].vertices.at(vertex) = 0;
	chains[chain_index].num_liberties--;
	return true;
}

void Board::capture_chain(uint16_t vertex) {
	uint8_t chain_index = chain_reps[vertex];
	assert(chain_index < chains.size()); //TODO this assertion keeps failing
	int side = (chains[chain_index].side == BLACK) ? 0 : 1;
	for (int i = 0; i < num_vertices; i++) {
		if (valid_vertex(i)) {
			if (chains[chain_index].vertices[i]) {
				remove_stone(i);
				num_prisoners[side]++;
			}
		}
	}
	delete_chain(chain_index);
}

void Board::delete_chain(uint8_t chain_index) {
	//delete &chains[chain_index];
	assert(chain_index < chains.size());
	chains.erase(chains.begin() + chain_index);
	for (uint8_t i = 0; i < chain_reps.size(); i++) {
		if (valid_vertex(i)) {
			if (chain_reps[i] == chain_index) {
				chain_reps[i] = 255;
			} else if (chain_reps[i] > chain_index && chain_reps[i] != 255) {
				chain_reps[i]--;
			}
		}
	}
}

bool Board::is_suicide(uint16_t vertex, bool side) const { //TODO test this
	assert(valid_vertex(vertex));
	if (liberties(vertex) > 0) { //if liberties != 0, has liberties, not suicide
		return false;
	}
	for (int i = 0; i < 4; i++) {
		int neighbor = vertex + directions[i];
		if (valid_vertex(neighbor)) {
			if (chain_reps[neighbor] != 255) {
				uint8_t chain_index = chain_reps[neighbor];
				if (chains[chain_index].side == side) {
					if (chains[chain_index].num_liberties >= 2) { //if adj to a chain of same color w liberties
						return false;
					}
				} else {
					if (chains[chain_index].num_liberties <= 1) { //if capturing an enemy chain, will have no liberties
						return false;
					}
				}
			}
		}
	}
	return true;
}

bool Board::is_eye(uint16_t vertex, bool side) const {
	assert(valid_vertex(vertex));
	if (get_state(vertex) != EMPTY) {
		return false;
	}
	vertex_t type = side ? BLACK : WHITE;
	vertex_t other = side ? WHITE : BLACK;
	if (get_neighbors(vertex, type) < (4 - get_neighbors(vertex, INVAL))) {
		return false;
	}

	if (get_neighbors(vertex, EMPTY) > 0) {
		return false;
	}

	int colorcount[4];

	colorcount[BLACK] = 0;
	colorcount[WHITE] = 0;
	colorcount[INVAL] = 0;
	colorcount[EMPTY] = 0;

//diagonal corners
	colorcount[board[vertex - 1 - (board_size + 2)]]++;
	colorcount[board[vertex + 1 - (board_size + 2)]]++;
	colorcount[board[vertex - 1 + (board_size + 2)]]++;
	colorcount[board[vertex + 1 + (board_size + 2)]]++;

	if (colorcount[INVAL] == 0) {
		if (colorcount[other] > 1) {
			return false;
		}
	} else {
		if (colorcount[other]) {
			return false;
		}
	}
	return true;
}

int Board::get_net_prisoners() const {
	return num_prisoners[0] - num_prisoners[1];

}

int Board::get_chain_liberties(uint16_t vertex) const {
	assert(valid_vertex(vertex));
	assert(board[vertex] != EMPTY);

	if (chain_reps[vertex] >= chains.size()) {
		//print_chains();
		printf("\n%d %d\n", vertex, chain_reps[vertex]);
	}
	uint8_t chain_index = chain_reps[vertex];
	return chains[chain_index].num_liberties;
}

