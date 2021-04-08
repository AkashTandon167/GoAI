#include "Board.h"
#include "Game.h"
#include "AI.h"
#include <windows.h>

#include <SFML/Graphics.hpp>

static sf::Texture board;
static sf::Texture stones[2];

void get_textures() {
	board.loadFromFile("Images/goban.png");
	stones[0].loadFromFile("Images/black_stone.png");
	stones[1].loadFromFile("Images/white_stone.png");
}

void play(Game g, int moves_ahead) {
	int size = g.get_size();
	int window_size = size * 30 + 2;
	sf::RenderWindow window(sf::VideoMode(window_size, window_size),
			"StellaChess");
	get_textures();
	while (window.isOpen() && g.ongoing()) {
		sf::Event e;
		while (window.pollEvent(e)) {
			if (e.type == sf::Event::Closed) {
				window.close();
			}
			int best_move = bestMove(g, moves_ahead);
			if (g.move(best_move)) {
				window.draw(sf::Sprite(board));
				for (int i = 0; i < size; i++) {
					for (int j = 0; j < size; j++) {
						sf::Sprite piece;
						Board::vertex_t current = g.get_state(i, j);
						if (current != Board::EMPTY) {
							piece.setTexture(stones[current == Board::WHITE]);
							piece.setPosition(30 * j + 1, 30 * i + 1);
							window.draw(piece);
						}
					}
				}
				window.display();
				Sleep(500);
				window.clear();
			}
		}
	}
}

void display(Game g) {
	int size = g.get_size();
	int window_size = size * 30 + 2;
	sf::RenderWindow window(sf::VideoMode(window_size, window_size),
			"StellaChess");
	get_textures();
	while (window.isOpen()) {
		sf::Event e;
		while (window.pollEvent(e)) {
			if (e.type == sf::Event::Closed) {
				window.close();
			}
			window.draw(sf::Sprite(board));
			for (int i = 0; i < size; i++) {
				for (int j = 0; j < size; j++) {
					sf::Sprite piece;
					Board::vertex_t current = g.get_state(i, j);
					if (current != Board::EMPTY) {
						piece.setTexture(stones[current == Board::WHITE]);
						piece.setPosition(30 * j + 1, 30 * i + 1);
						window.draw(piece);
					}
				}
			}
			window.display();
		}
	}
}

int main() {
	srand(1);
	int size = 19;
	Game x = Game(size);
	int i = 0;
	while (x.ongoing()) {
		int best_move = bestMove(x, 2);
		printf("%d\n", best_move);
		if (x.move(best_move)) {
			i++;
			x.print();
		}
	}

	return 0;
}
