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
	GameState(GameState&& other) = delete;
	GameState& operator=(GameState&& other) = delete;

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
	char GetCurrentPlayer() const { if (m_P1Turn) return m_Player1; else return m_Player2; };
	char GetWaitingPlayer() const { if (!m_P1Turn) return m_Player2; else return m_Player1; };
	char GetOpponentPiece(const char& myPiece) { if (myPiece == m_Player1) return m_Player1; else return m_Player2; };
protected:
	std::array<std::array<char, 7>, 6> m_Board{};
	int m_LastMove{ INVALID_INDEX };
	bool m_P1Turn{ true };

	int m_NrPieces{ 0 };
	char m_Player1;
	char m_Player2;
};

