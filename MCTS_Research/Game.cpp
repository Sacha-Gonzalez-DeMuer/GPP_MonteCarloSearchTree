#include "pch.h"
#include "Game.h"
#include "utils.h"
#include "Board.h"
#include "Player.h"
#include <iostream>
#include "MonteCarloTreeSearch.h"
#include "C4Analysis.h"
#include "Texture.h"

Game::Game(const Window& window)
	: m_Window{ window }
	, m_pPlayer1{ new Player(PLAYER1, true, "Sacha") }
	, m_pPlayer2{ new Player(PLAYER2, false, "Carlos") }
	, m_pStateAnalysis{ new C4_Analysis() }
	, m_pPlayer1TurnTxt{ new Texture(m_pPlayer1->GetName() + "'s turn", "Resources/crux.ttf", 40, PLAYER1) }
	, m_pPlayer2TurnTxt{ new Texture(m_pPlayer2->GetName() + "' turn",  "Resources/crux.ttf", 40, PLAYER2) }
	, m_pPlayer1WinTxt{ new Texture(m_pPlayer1->GetName() + " wins!",  "Resources/crux.ttf", 40, PLAYER1) }
	, m_pPlayer2WinTxt{ new Texture(m_pPlayer2->GetName() + " wins!",  "Resources/crux.ttf", 40, PLAYER2) }

{
	m_pBoard = new Board(50.0f, window, m_pPlayer1, m_pPlayer2);
	Initialize();
}

Game::~Game()
{
	Cleanup();
}

void Game::Initialize()
{

}

void Game::Cleanup()
{
	delete m_pBoard;
	m_pBoard = nullptr;

	delete m_pPlayer1;
	delete m_pPlayer2;
	m_pPlayer1 = nullptr;
	m_pPlayer2 = nullptr;

	delete m_pPlayer1TurnTxt;
	delete m_pPlayer2TurnTxt;
	delete m_pPlayer1WinTxt;
	delete m_pPlayer2WinTxt;
	m_pPlayer1TurnTxt = nullptr;
	m_pPlayer2TurnTxt = nullptr;
	m_pPlayer1WinTxt = nullptr;
	m_pPlayer2WinTxt = nullptr;
}

void Game::Update(float elapsedSec)
{
	Player* current_player{ m_FirstPlayerTurn ? m_pPlayer1 : m_pPlayer2 };
	int move{};
	if (!m_GameFinished && current_player->GetMove(*m_pBoard, move))
	{
		if (!m_pBoard->PlacePiece(move, current_player->GetInitial()))
		{
			std::cout << "Column full\n";
			return;
		}

		if (m_pStateAnalysis->CheckWin(*m_pBoard, current_player->GetInitial()))
		{
			std::cout << current_player->GetName() << " wins!\n";
			m_GameFinished = true;
			return;
		}

		if (m_pStateAnalysis->CheckDraw(*m_pBoard))
		{
			std::cout << "Draw!\n";
			return;
		}

		m_FirstPlayerTurn = !m_FirstPlayerTurn;
	}
}

void Game::Draw() const
{
	ClearBackground();

	Rectf dst_rect{  };
	Point2f text_pos{ m_Window.width / 2, 20 };
	if (m_FirstPlayerTurn && !m_GameFinished)
	{
		dst_rect.width = m_pPlayer1TurnTxt->GetWidth();
		dst_rect.height = m_pPlayer1TurnTxt->GetHeight();
		text_pos.x -= dst_rect.width / 2;
		m_pPlayer1TurnTxt->Draw(text_pos, dst_rect);
	}
	else if (!m_FirstPlayerTurn && !m_GameFinished)
	{
		dst_rect.width = m_pPlayer2TurnTxt->GetWidth();
		dst_rect.height = m_pPlayer2TurnTxt->GetHeight();
		text_pos.x -= dst_rect.width / 2;
		m_pPlayer2TurnTxt->Draw(text_pos, dst_rect);
	}
	else if (m_FirstPlayerTurn && m_GameFinished)
	{
		dst_rect.width = m_pPlayer1WinTxt->GetWidth();
		dst_rect.height = m_pPlayer1WinTxt->GetHeight();
		text_pos.x -= dst_rect.width / 2;
		m_pPlayer1WinTxt->Draw(text_pos, dst_rect);
	}
	else if (!m_FirstPlayerTurn && m_GameFinished)
	{
		dst_rect.width = m_pPlayer2WinTxt->GetWidth();
		dst_rect.height = m_pPlayer2WinTxt->GetHeight();
		text_pos.x -= dst_rect.width / 2;
		m_pPlayer2WinTxt->Draw(text_pos, dst_rect);
	}
	m_pBoard->Render();
}

void Game::ProcessKeyDownEvent(const SDL_KeyboardEvent& e)
{
	switch (e.keysym.sym)
	{
	case SDLK_r:
		ResetGame();
		break;
	}
}

void Game::ProcessKeyUpEvent(const SDL_KeyboardEvent& e)
{
}

void Game::ProcessMouseMotionEvent(const SDL_MouseMotionEvent& e)
{
}

void Game::ProcessMouseDownEvent(const SDL_MouseButtonEvent& e)
{
	if (m_FirstPlayerTurn)
		m_pPlayer1->ProcessMouseDownEvent(e);
	else
		m_pPlayer2->ProcessMouseDownEvent(e);

	if (m_GameFinished)
		ResetGame();
}

void Game::ProcessMouseUpEvent(const SDL_MouseButtonEvent& e)
{

}

void Game::ClearBackground() const
{
	glClearColor(0.0f, 0.0f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
}

void Game::ResetGame()
{
	m_pBoard->Reset();
	m_GameFinished = false;
	m_FirstPlayerTurn = true;
	m_pPlayer1->Reset();
	m_pPlayer2->Reset();
}
