#pragma once
#include "StateAnalysis.h"
#include <array>
class GameState
{
public:
	GameState();
	GameState(char player1, char player2);
	GameState(const GameState& other);
	GameState& operator=(const GameState& other);

	void Initialize();
	void Reset();

	bool PlacePiece(const int& column, const char& player);

	//Getters
	std::array<std::array<char, 7>, 6> GetBoard() const { return m_Board; };
	int GetLastMove() const { return m_LastMove; };
	int GetNrRows() const { return static_cast<int>(m_Board.size()); };
	int GetNrColumns() const { return static_cast<int>(m_Board[0].size()); };
	int GetNrPieces() const { return m_NrPieces; };
	char GetP1Piece() { return m_Player1; };
	char GetP2Piece() { return m_Player2; };
	bool IsPlayer1Turn() const { return m_P1Turn; };
	bool IsPlayerTurn(const char& player) { return (m_P1Turn && player == m_Player1); };

protected:
	std::array<std::array<char, 7>, 6> m_Board{};
	int m_LastMove{ INVALID_INDEX };
	bool m_P1Turn{ true };

	int m_NrPieces{ 0 };
	char m_Player1;
	char m_Player2;
};

