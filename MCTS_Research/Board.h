#pragma once
#include "GameState.h"
#include "C4Analysis.h"

class Player;

class Board final : public GameState
{
public:
	Board();
	Board(float cellSize, const Window& window, Player* player1, Player* player2);
	Board(const Board& other);
	Board(Board&& other) = delete;
	Board& operator=(Board&& other) = delete;
	Board& operator=(const Board& other) = delete;
	~Board();

	void Render() const;

	// Getters
	Rectf GetBoardRect() const { return m_BoardRect; };
	float GetCellSize() const { return m_CellSize; };
	std::array<std::array<char, 7>, 6> GetBoard() const { return m_Board; };

private:
	Rectf m_BoardRect;
	float m_CellSize{50.0f};
 	Color4f m_BoardColor{ 0.f, .5f, 1.0f, 1.0f };
	Player* m_pPlayer1;
	Player* m_pPlayer2;
};