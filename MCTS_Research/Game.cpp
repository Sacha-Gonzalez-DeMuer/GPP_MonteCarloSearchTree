#include "pch.h"
#include "Game.h"
#include "utils.h"
#include "Board.h"
#include "Player.h"
#include <iostream>
#include "MonteCarloTreeSearch.h"
#include "C4Analysis.h"

Game::Game( const Window& window ) 
	: m_Window{ window }
	, m_pPlayer1{new Player(PLAYER1, false, "Giuseppe")}
	, m_pPlayer2{new Player(PLAYER2, false, "Carlos")}
	, m_pStateAnalysis{new C4_Analysis()}
{
	m_pBoard = new Board(50.0f, window, m_pPlayer1, m_pPlayer2);
	Initialize( );
}

Game::~Game( )
{
	Cleanup( );
}

void Game::Initialize( )
{
	
}

void Game::Cleanup( )
{
	delete m_pBoard;
	m_pBoard = nullptr;

	delete m_pPlayer1;
	delete m_pPlayer2;
	m_pPlayer1 = nullptr;
	m_pPlayer2 = nullptr;
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

void Game::Draw( ) const
{
	ClearBackground( );

	m_pBoard->Render();
}

void Game::ProcessKeyDownEvent( const SDL_KeyboardEvent & e )
{
	switch (e.keysym.sym)
	{
	case SDLK_r:
		ResetGame();
		break;
	}
}

void Game::ProcessKeyUpEvent( const SDL_KeyboardEvent& e )
{
}

void Game::ProcessMouseMotionEvent( const SDL_MouseMotionEvent& e )
{
}

void Game::ProcessMouseDownEvent( const SDL_MouseButtonEvent& e )
{
	if(m_FirstPlayerTurn)
		m_pPlayer1->ProcessMouseDownEvent(e);
	else
		m_pPlayer2->ProcessMouseDownEvent(e);

	if (m_GameFinished)
		ResetGame();
}

void Game::ProcessMouseUpEvent( const SDL_MouseButtonEvent& e )
{
	
}

void Game::ClearBackground( ) const
{
	glClearColor( 0.0f, 0.0f, 0.3f, 1.0f );
	glClear( GL_COLOR_BUFFER_BIT );
}

void Game::ResetGame()
{
	m_pBoard->Reset();
	m_GameFinished = false;
	m_FirstPlayerTurn = true;
	m_pPlayer1->Reset();
	m_pPlayer2->Reset();
}
