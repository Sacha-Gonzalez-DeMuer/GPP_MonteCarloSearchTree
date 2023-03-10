#pragma once
#include <memory>

class Player;
class Board;
class Texture;
struct StateAnalysis;

class Game final
{
public:
	explicit Game( const Window& window );
	Game(const Game& other) = delete;
	Game& operator=(const Game& other) = delete;
	Game( Game&& other) = delete;
	Game& operator=(Game&& other) = delete;
	~Game();

	void Update( float elapsedSec );
	void Draw( ) const;

	// Event handling
	void ProcessKeyDownEvent( const SDL_KeyboardEvent& e );
	void ProcessKeyUpEvent( const SDL_KeyboardEvent& e );
	void ProcessMouseMotionEvent( const SDL_MouseMotionEvent& e );
	void ProcessMouseDownEvent( const SDL_MouseButtonEvent& e );
	void ProcessMouseUpEvent( const SDL_MouseButtonEvent& e );

private:
	// DATA MEMBERS
	const Window m_Window;

	// FUNCTIONS
	void Initialize( );
	void Cleanup( );
	void ClearBackground( ) const;
	void ResetGame();

	Board* m_pBoard;
	StateAnalysis* m_pStateAnalysis;
	Player* m_pPlayer1;
	Player* m_pPlayer2;

	Texture* m_pPlayer1TurnTxt;
	Texture* m_pPlayer2TurnTxt;
	Texture* m_pPlayer1WinTxt;
	Texture* m_pPlayer2WinTxt;

	bool m_FirstPlayerTurn{ true };
	bool m_GameFinished{ false };
};