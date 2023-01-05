#pragma once
#include <array>

#define EMPTY Color4f(0,0,0,1)

class Board {
public:
	// Constructor
	Board(float cellSize, const Window& window);

	void Render() const;

	// Places a piece of the given color at the given column. 
	// True if the move was successful, false if the column is full.
	bool PlacePiece(int column, const Color4f& color);

	// True if the given player has won the game, false otherwise.
	bool CheckWin(const Color4f& color);

	// True if the board is full and the game is a draw, false otherwise.
	bool CheckDraw();

	void Reset();

	// Getters
	Rectf GetBoardRect() const { return m_BoardRect; };
	float GetCellSize() const { return m_CellSize; };

private:

	// The number of pieces that have been placed on the board.
	int m_NrPieces;
	int m_NrRows;
	int m_NrColumns;
	Rectf m_BoardRect;

	// Corresponds to m_Board[rows][columns]
	std::array<std::array<Color4f, 7>, 6> m_Board{};
	float m_CellSize;

	Color4f m_BoardColor{ 0.f, .5f, 1.0f, 1.0f };
};