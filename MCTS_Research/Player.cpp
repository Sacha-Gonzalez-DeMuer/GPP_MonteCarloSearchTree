#include "pch.h"
#include "Player.h"
#include <iostream>
#include "Board.h"
#include "MonteCarloTreeSearch.h"

Player::Player(const Color4f& color, bool isHuman, const std::string& name)
	: m_Color{ color }
	, m_IsHuman{ isHuman }
	, m_Name{ name }
	, m_pMCTS{ new MonteCarloTreeSearch(this) }
{
}

Player::~Player()
{
	delete m_pMCTS;
	m_pMCTS = nullptr;
}

bool Player::GetMove(const Board& pBoard, int& i)
{
	if (!m_WaitingForMove) m_WaitingForMove = true;

	if (m_IsHuman)
	{
		// Check if player has clicked && on the board
		if (m_ClickPos != INVALID_POSITION && utils::IsPointInRect(m_ClickPos, pBoard.GetBoardRect()))
		{
			// Transform click to board space to prepare for index calculation
			m_ClickPos.x -= pBoard.GetBoardRect().left;

			// Assign the column index to i.
			i = static_cast<int>(m_ClickPos.x / pBoard.GetCellSize());

			// Reset m_ClickPos and m_WaitingForMove.
			m_ClickPos = INVALID_POSITION;
			m_WaitingForMove = false;

			return true;
		}
	}
	else {
		//Monte Carlo Tree Search
		i = m_pMCTS->FindNextMove(pBoard);
		return true;
	}

	return false;
}

void Player::ProcessMouseDownEvent(const SDL_MouseButtonEvent& e)
{
	if (!m_WaitingForMove)
	{
		m_ClickPos = INVALID_POSITION;
		return;
	}


	switch (e.button)
	{
		case SDL_BUTTON_LEFT:
			m_ClickPos.x = static_cast<float>(e.x);
			m_ClickPos.y = static_cast<float>(e.y);

			break;
	}
}

void Player::Reset()
{

	delete m_pMCTS;
	m_pMCTS = nullptr;

	m_pMCTS = new MonteCarloTreeSearch(this);
	m_WaitingForMove = false;
}
