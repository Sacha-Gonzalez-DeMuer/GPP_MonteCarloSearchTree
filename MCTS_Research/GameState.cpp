#pragma once
#include "pch.h"
#include "GameState.h"
#include <iostream>

GameState::GameState()
{
    Initialize();
}

GameState::GameState(char player1, char player2)
    : m_Player1{player1}
    , m_Player2{player2}
{
    Initialize();
}

GameState::GameState(const GameState& other)
    : m_Board{ other.m_Board }
    , m_LastMove{ other.m_LastMove }
    , m_P1Turn{ other.m_P1Turn }
    , m_NrPieces{other.m_NrPieces}
    , m_Player1{other.m_Player1}
    , m_Player2{other.m_Player2}
{
}

GameState& GameState::operator=(const GameState& other)
{
    m_NrPieces = other.m_NrPieces;
    m_LastMove = other.m_LastMove;
    m_Board = other.m_Board;
    m_P1Turn = other.m_P1Turn;
    m_LastMove = other.m_LastMove;
    m_Player1 = other.m_Player1;
    m_Player2 = other.m_Player2;
    return *this;
}

void GameState::Initialize()
{
    for (int row{ 0 }; row < GetNrRows(); ++row)
        for (int col{ 0 }; col < GetNrColumns(); ++col)
            m_Board[row][col] = EMPTY;
}


void GameState::Reset()
{
    m_P1Turn = true;
    m_NrPieces = 0;
    m_LastMove = INVALID_INDEX;
    Initialize();
}

bool GameState::PlacePiece(const int& column, const char& player)
{
    // Catch player on wrong turn
    if (m_P1Turn && player != m_Player1)
    {
        std::cerr << "NOT YOUR TURN!\n";
        return false;
    }

    // Check if the column is full.
    if (m_Board[GetNrRows() - 1][column] != EMPTY) {
        return false;
    }

    // Find the lowest empty cell in the column.
    int row = 5;
    while (row >= 0 && m_Board[row][column] == EMPTY) {
        --row;
    }

    if (row + 1 < GetNrRows() && row + 1 >= 0
        && column < GetNrColumns() && column >= 0)
    {
        // Place the piece in the cell.
        m_Board[row + 1][column] = player;
        m_LastMove = column;
        ++m_NrPieces;
        m_P1Turn = !m_P1Turn;

        return true;
    }

    return false;
}
