#include "pch.h"
#include "Board.h"
#include <iostream>
#include "Player.h"

Board::Board()
    : GameState(EMPTY, EMPTY)
    , m_CellSize{ 50.0f }
    , m_BoardRect{  }
    , m_pPlayer1{nullptr}
    , m_pPlayer2{nullptr}
{
    m_BoardRect.width = GetNrColumns() * m_CellSize;
    m_BoardRect.height = GetNrRows() * m_CellSize;
}

Board::Board(float cellSize, const Window& window, Player* player1, Player* player2)
    : GameState(player1->GetInitial(), player2->GetInitial())
    , m_CellSize{ cellSize }
    , m_BoardRect{  }
    , m_pPlayer1{ player1 }
    , m_pPlayer2{ player2 }
{
    m_BoardRect.width = GetNrColumns() * m_CellSize;
    m_BoardRect.height = GetNrRows() * m_CellSize;
    m_BoardRect.left = window.width / 2.0f - m_BoardRect.width / 2.0f;
    m_BoardRect.bottom = window.height / 2.0f - m_BoardRect.height / 2.0f;
}


Board::Board(const Board& other)
    : GameState(other)
    , m_BoardRect{ other.m_BoardRect }
    , m_BoardColor{ other.m_BoardColor }
    , m_CellSize{other.m_CellSize}
    , m_pPlayer1{other.m_pPlayer1}
    , m_pPlayer2{other.m_pPlayer2}
{
}

Board::~Board()
{
    m_pPlayer1 = nullptr;
    m_pPlayer2 = nullptr;
}

void Board::Render() const
{
    utils::SetColor(m_BoardColor);
    utils::FillRect(GetBoardRect());

    Color4f piece_color{};
    float circle_size{ m_CellSize * .5f };
    // Fix the position of the ellipse for the first row.
    Ellipsef cell{ GetBoardRect().left + circle_size,
                   GetBoardRect().bottom + circle_size,
                   m_CellSize * .5f,
                   m_CellSize * .5f };
    for (int row = 0; row < GetNrRows(); ++row)
    {
        for (int col = 0; col < GetNrColumns(); ++col)
        {
            // Use color of correct players piece
            char cell_occupation{ m_Board[row][col] };

            if (cell_occupation == m_pPlayer1->GetInitial())
                piece_color = m_pPlayer1->GetColor();
            else if (cell_occupation == m_pPlayer2->GetInitial())
                piece_color = m_pPlayer2->GetColor();
            else if (cell_occupation == EMPTY)
                piece_color = Color4f{ 0,0,0,1 };
            else
            {
                std::cerr << "invalid cell occupation\n";
            }

            utils::SetColor(piece_color);
            utils::FillEllipse(cell);
            cell.center.x += m_CellSize;
        }
        // Reset the x position of the ellipse for the next row.
        cell.center.x = GetBoardRect().left + circle_size;
        cell.center.y += m_CellSize;
    }
}


