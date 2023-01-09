#include "pch.h"
#include "MonteCarloTreeSearch.h"
#include <random>
#include "Board.h"
#include <iostream>
MonteCarloTreeSearch::MonteCarloTreeSearch()
{
}

MonteCarloTreeSearch::~MonteCarloTreeSearch()
{
	for (auto& child : m_RootNode->Children)
	{
		delete child;
		child = nullptr;

	}

	m_RootNode->Children.clear();
	delete m_RootNode;
	m_RootNode = nullptr;
}


int MonteCarloTreeSearch::FindNextMove(const Board& pBoard)
{
	m_RootNode = new MCTSNode(pBoard);
	MCTSNode* root{ m_RootNode };
	MCTSNode* promising_node{ root };
	for (int i = 0; i < m_NrIterations; i++)
	{
		// Select a node with highest Upper Confidence Boundary
		promising_node = SelectNode(root);

		// If state of node isn't complete, make a new child node for all possible moves
		if (promising_node->State.InProgress())
			Expand(promising_node);

		// Choose a random child to run simulations on
		MCTSNode* node_to_explore{ promising_node };
		if (!promising_node->Children.empty())
		{
			int rnd_int{ utils::GetRandomInt(static_cast<int>(promising_node->Children.size())) };
			node_to_explore = promising_node->Children[rnd_int];

		}

		BackPropagate(node_to_explore, Simulate(node_to_explore));
	}


	//Find node with most wins? || UCB?
	MCTSNode* best_node{ nullptr };
	for (const auto& child : m_RootNode->Children)
	{
		if (!best_node)
		{
			best_node = child;
			continue;
		}

		if (child->VisitCount > best_node->VisitCount)
			best_node = child;
	}

	int move{ -1 };
	if(best_node)
		 move = best_node->State.GetLastMove();


	delete m_RootNode;
	m_RootNode = nullptr;

	return move;
}

MCTSNode* MonteCarloTreeSearch::SelectNode(MCTSNode* fromNode)
{
	//Start
	MCTSNode* current_node{ fromNode };

	// Find leaf node with maximum win rate
	while (!current_node->IsLeaf())
	{
		MCTSNode* highest_UCD_node{};
		// Find child node that maximises Upper Confidence Boundary
		for (auto& child : current_node->Children)
		{
			// On first iteration set highest to first child node
			if (!highest_UCD_node)
			{
				highest_UCD_node = child;
				continue;
			}

			if (CalculateUCB(*child) > CalculateUCB(*highest_UCD_node))
				highest_UCD_node = child;
		}

		current_node = highest_UCD_node;
	}

	return current_node;
}

void MonteCarloTreeSearch::Expand(MCTSNode*& fromNode)
{
	// For each available action from current state, add new state to the tree
	const auto& available_actions{ fromNode->State.GetAvailableActions() };

	std::vector<MCTSNode*> new_children{};
	for (const auto& action : available_actions)
	{
		// Make a copy of the board state
		Board new_state{ fromNode->State };

		// Play the available action
		new_state.PlacePiece(action, myColor);

		// Create new child node
		MCTSNode* new_node{ new MCTSNode(new_state) };
		new_node->Parent = fromNode;
		new_children.push_back(new_node);
	}

	// Add newly generated children to the node
	for (auto new_child : new_children)
	{
		fromNode->Children.emplace_back(new_child);
	}
}


/* After Expansion, the algorithm picks a child node arbitrarily,
and it simulates a randomized game from selected node until it reaches the resulting state of the game.*/
//Simulate game on node randomly, returns winner color if there is one, empty color if draw
Color4f MonteCarloTreeSearch::Simulate(MCTSNode* node)
{
	Color4f current_player{ PLAYER1 };
	Color4f opponent_player{ myColor };
	Board state_copy{ node->State };

	// Loop forever
	while (true)
	{
		if (state_copy.IsPlayer1Turn())
		{
			current_player = PLAYER1;
			opponent_player = PLAYER2;
		}
		else
		{
			current_player = PLAYER2;
			opponent_player = PLAYER1;
		}

		bool has_moved{ false };
		int rnd_move_idx{ -1 };


		// Check if self has a winning move and play it
		const auto& winning_connect4_moves{ GetCompletingCellsIndices(state_copy, current_player, 4) };
		rnd_move_idx = utils::GetRandomInt(static_cast<int>(winning_connect4_moves.size()));
		if (!winning_connect4_moves.empty()
			&& state_copy.PlacePiece(winning_connect4_moves[rnd_move_idx], current_player))
		{
			has_moved = true;
		}

		// Check if opponent has winning move and counter it
		if (!has_moved)
		{
			const auto& opponent_connect4_moves{ GetCompletingCellsIndices(state_copy, opponent_player, 4) };
			rnd_move_idx = utils::GetRandomInt(static_cast<int>(opponent_connect4_moves.size()));
			if (!opponent_connect4_moves.empty()
				&& state_copy.PlacePiece(opponent_connect4_moves[rnd_move_idx], current_player))
			{
				has_moved = true;
			}
		}

		// Play random move
		if (!has_moved)
		{
			const auto available_actions{ state_copy.GetAvailableActions() };
			state_copy.PlacePiece(available_actions[utils::GetRandomInt(static_cast<int>(available_actions.size()))], current_player);
		}

		if (state_copy.CheckWin(myColor))
			return myColor;

		if (state_copy.CheckWin(PLAYER1))
			return PLAYER1;

		if (state_copy.CheckDraw())
			return EMPTY;
	}
}


/*
 Once the algorithm reaches the end of the game,
 it evaluates the state to figure out which player has won.
 It traverses upwards to the root and increments visit score for all visited nodes.
 It also updates win score for each node if the player for that position has won the playout.
*/
void MonteCarloTreeSearch::BackPropagate(MCTSNode* fromNode, Color4f winningPlayer)
{
	MCTSNode* current_node{ fromNode };
	int reward{ 0 };

	// If its player 1 turn it means this node played on my turn
	if (fromNode->State.IsPlayer1Turn() && winningPlayer == myColor)
		reward = 1;


	while (current_node != nullptr)
	{
		++current_node->VisitCount;

		if (winningPlayer == myColor)
			current_node->WinCount += reward;

		current_node = current_node->Parent;

		if (winningPlayer == EMPTY)
			reward = 0;
		else
			reward = 1 - reward;

	};
}

std::vector<int> MonteCarloTreeSearch::GetCompletingCellsIndices(const Board& board, const Color4f& color, int piecesInARow) const
{
	std::vector<int> completingColumnsIndices;

	// Check if theres a sequence of piecesInARow-1 
	if (board.CheckPiecesInAHorizontalRow(color, piecesInARow-1))
	{
		// Check if can be completed (check if there's a piece below the needed position)

		// Get the almost completed row
		std::pair<BoardPosition, BoardPosition> horizontalRow{ GetHorizontalRowStartAndEnd(board, color, piecesInARow - 1) };

		if (horizontalRow.first != INVALID_BOARD_POSITION || horizontalRow.second != INVALID_BOARD_POSITION)
		{
			// Check if you can place a piece in the positions next to it
			if (horizontalRow.first.column - 1 > 0
				&& IsEmptyWitFullCellBelow(board, horizontalRow.first.row, horizontalRow.first.column - 1)) //check left side
			{
				completingColumnsIndices.push_back(horizontalRow.first.column - 1); //save that move
			}
			if (horizontalRow.second.column + 1 < board.GetNrColumns()
				&& IsEmptyWitFullCellBelow(board, horizontalRow.first.row, horizontalRow.first.column + 1)) //check right side
			{
				completingColumnsIndices.push_back(horizontalRow.second.column + 1);
			}
		}
	}

	if (board.CheckPiecesInAVerticalRow(color, piecesInARow-1))
	{
		// Get the almost completed row
		std::pair<BoardPosition, BoardPosition> verticalRow{ GetVerticalRowStartAndEnd(board, color, piecesInARow - 1) };

		if (verticalRow.first != INVALID_BOARD_POSITION || verticalRow.second != INVALID_BOARD_POSITION)
		{
			// Check if you can place a piece on top of it
			if (verticalRow.second.row + 1 < board.GetNrRows()
				&& IsEmptyWitFullCellBelow(board, verticalRow.second.row + 1, verticalRow.second.column)) //check left side
			{
				completingColumnsIndices.push_back(verticalRow.second.column); //save that move
			}
		}
	}

	if (board.CheckPiecesInADiagonalRow(color, piecesInARow-1, true))
	{
		std::pair<BoardPosition, BoardPosition> diagonalRow{ GetDiagonalRowStartAndEnd(board, color, piecesInARow - 1, true) };

		if (diagonalRow.first != INVALID_BOARD_POSITION || diagonalRow.second != INVALID_BOARD_POSITION)
		{
			// Check if you can place a piece in the positions next to it
			if (diagonalRow.first.column - 1 > 0 && diagonalRow.first.row - 1 > 0
				&& IsEmptyWitFullCellBelow(board, diagonalRow.first.row - 1, diagonalRow.first.column - 1))
			{
				completingColumnsIndices.push_back(diagonalRow.first.column - 1);
			}

			if (diagonalRow.second.column + 1 < board.GetNrColumns() && diagonalRow.second.row + 1 < board.GetNrRows()
				&& IsEmptyWitFullCellBelow(board, diagonalRow.first.row + 1, diagonalRow.first.column + 1))
			{
				completingColumnsIndices.push_back(diagonalRow.first.column + 1);
			}
		}
	}

	if (board.CheckPiecesInADiagonalRow(color, piecesInARow-1, false))
	{
		std::pair<BoardPosition, BoardPosition> diagonalRow{ GetDiagonalRowStartAndEnd(board, color, piecesInARow - 1, false) };

		if (diagonalRow.first != INVALID_BOARD_POSITION || diagonalRow.second != INVALID_BOARD_POSITION)
		{
			// Check if you can place a piece in the positions next to it
			if (diagonalRow.first.column - 1 > 0 && diagonalRow.first.row - 1 > 0
				&& IsEmptyWitFullCellBelow(board, diagonalRow.first.row - 1, diagonalRow.first.column - 1))
			{
				completingColumnsIndices.push_back(diagonalRow.first.column - 1);
			}

			if (diagonalRow.second.column + 1 < board.GetNrColumns() && diagonalRow.second.row + 1 < board.GetNrRows()
				&& IsEmptyWitFullCellBelow(board, diagonalRow.second.row + 1, diagonalRow.second.column + 1))
			{
				completingColumnsIndices.push_back(diagonalRow.second.column + 1);
			}
		}
	}

	return completingColumnsIndices;
}

bool MonteCarloTreeSearch::IsEmptyWitFullCellBelow(const Board& board, int row, int column) const
{
	//Check if cell is empty
	if (board.GetBoard()[row][column] != EMPTY) {
		return false;
	}

	//Check if cell below is full
	if (row - 1 < 0 || board.GetBoard()[row - 1][column] != EMPTY)
		return true;

	return false;
}

std::pair<BoardPosition, BoardPosition> MonteCarloTreeSearch::GetHorizontalRowStartAndEnd(const Board& board, const Color4f& color, int piecesInARow) const
{
	BoardPosition startPos{ -1, -1 };
	BoardPosition endPos{ -1,-1 };

	// Check if connected horizontally
	for (int row = 0; row < board.GetNrRows(); row++) {
		for (int col = 0; col < board.GetNrColumns() - piecesInARow + 1; col++)
		{
			bool connected{ true };
			startPos.row = row;
			startPos.column = col;

			for (int i = 0; i < piecesInARow; i++) 
			{
				if (board.GetBoard()[row][col + i] != color) 
				{
					connected = false;
					break;
				}
			}
			if (connected)
			{
				endPos.column = col + piecesInARow-1;
				return std::pair<BoardPosition, BoardPosition>(startPos, endPos);
			}
		}
	}

	return std::pair<BoardPosition, BoardPosition>(INVALID_BOARD_POSITION, INVALID_BOARD_POSITION);
}

std::pair<BoardPosition, BoardPosition> MonteCarloTreeSearch::GetVerticalRowStartAndEnd(const Board& board, const Color4f& color, int piecesInARow) const
{
	BoardPosition startPos{ -1, -1 };
	BoardPosition endPos{ -1,-1 };

	// Check if connected vertically
	for (int row = 0; row < board.GetNrRows() - piecesInARow; row++) {
		for (int col = 0; col < board.GetNrColumns(); col++)
		{
			bool connected{ true };
			startPos.row = row;
			startPos.column = col;

			for (int i = 0; i < piecesInARow; i++) 
			{
				if (board.GetBoard()[row + i][col] != color) {
					connected = false;
					break;
				}
			}

			if (connected)
			{
				endPos.row = row + piecesInARow;
				endPos.column = col;

				return std::pair<BoardPosition, BoardPosition>(startPos, endPos);
			}
		}
	}

	return std::pair<BoardPosition, BoardPosition>(INVALID_BOARD_POSITION, INVALID_BOARD_POSITION);
}

std::pair<BoardPosition, BoardPosition> MonteCarloTreeSearch::GetDiagonalRowStartAndEnd(const Board& board, const Color4f& color, int piecesInARow, bool ascending) const
{
	BoardPosition startPos{ -1, -1 };
	BoardPosition endPos{ -1,-1 };

	if (ascending)
	{
		// Check if connected diagonally (top-right to bottom-left)
		for (int row = 0; row < board.GetNrRows() - piecesInARow + 1; row++) {
			for (int col = 0; col < board.GetNrColumns() - piecesInARow + 1; col++)
			{
				bool connected{ true };
				startPos.row = row;
				startPos.column = col;

				for (int i = 0; i < piecesInARow; i++) 
				{
					if (board.GetBoard()[row + i][col + i] != color) 
					{
						connected = false;
						break;
					}
				}

				if (connected)
				{
					endPos.row = row + piecesInARow;
					endPos.column = col + piecesInARow;
					return std::pair<BoardPosition, BoardPosition>(startPos, endPos);
				}
			}
		}
	}
	else
	{
		// Check if connected diagonally (top-right to bottom-left)
		for (int row = 0; row < board.GetNrRows() - piecesInARow + 1; row++) {
			for (int col = piecesInARow - 1; col < board.GetNrColumns(); col++)
			{
				bool connected{ true };
				startPos.row = row;
				startPos.column = col;

				for (int i = 0; i < piecesInARow; i++)
				{
					if (board.GetBoard()[row + i][col - i] != color)
					{
						connected = false;
						break;
					}
				}

				if (connected)
				{
					endPos.row = row + piecesInARow;
					endPos.column = col - piecesInARow;
					return std::pair<BoardPosition, BoardPosition>(startPos, endPos);
				}
			}
		}
	}

	return std::pair<BoardPosition, BoardPosition>(INVALID_BOARD_POSITION, INVALID_BOARD_POSITION);
}

float MonteCarloTreeSearch::CalculateUCB(const MCTSNode& node) const
{
	if (node.VisitCount == 0)
		return FLT_MAX;

	float UCB{ 0 };
	// Calculate Exploitation
	UCB += static_cast<float>(node.WinCount) / static_cast<float>(node.VisitCount);

	// Calculate Exploration
	UCB += 2 * sqrtf(static_cast<float>(m_RootNode->VisitCount) / static_cast<float>(node.VisitCount)); 

	return UCB;
}

float MonteCarloTreeSearch::EvaluatePosition(const Board& board, const Color4f& forPlayer, const Color4f& againstPlayer) const
{
	float eval{ 0 };
	if (board.CheckWin(forPlayer))
		return FLT_MAX;

	if (board.CheckWin(againstPlayer))
		return FLT_MIN;

	if (board.CheckDraw())
		return 0;

	/*	const auto& winning_connect4_moves{ GetCompletingCellsIndices(board, forPlayer, 4) };
		const auto& losing_connect4_moves{ GetCompletingCellsIndices(board, againstPlayer, 4) };*/


		//eval += winning_connect4_moves.size();
		//eval -= losing_connect4_moves.size();

	// Add value for each winning move and remove for each winning move that the opponent has
	// Multiply by I to scale along with length of connection

	for (int i{ 0 }; i < 4; ++i)
	{
		eval += GetNrVerticalRows(board, forPlayer, i);
		eval -= GetNrVerticalRows(board, againstPlayer, i);

		eval += GetNrHorizontalRows(board, forPlayer, i);
		eval -= GetNrHorizontalRows(board, againstPlayer, i);

		eval += GetNrDiagonalRows(board, forPlayer, i);
		eval -= GetNrDiagonalRows(board, againstPlayer, i);

	}


	return eval;
}

int MonteCarloTreeSearch::GetNrHorizontalRows(const Board& board, const Color4f player, int piecesInARow) const
{
	int nr{ 0 };
	// Check if connected horizontally
	for (int row = 0; row < board.GetNrRows(); row++) {
		for (int col = 0; col < board.GetNrColumns() - piecesInARow + 1; col++)
		{
			bool connected{ true };
			for (int i = 0; i < piecesInARow; i++) {
				if (board.GetBoard()[row][col + i] != player) {
					connected = false;
					break;
				}
			}

			if (connected)
				++nr;
		}
	}

	return nr;
}

int MonteCarloTreeSearch::GetNrVerticalRows(const Board& board, const Color4f player, int piecesInARow) const
{
	int nr{ 0 };

	// Check if connected vertically
	for (int row = 0; row < board.GetNrRows() - piecesInARow + 1; row++) {
		for (int col = 0; col < board.GetNrColumns(); col++)
		{
			bool connected{ true };
			for (int i = 0; i < piecesInARow; i++)
			{
				if (board.GetBoard()[row + i][col] != player)
				{
					connected = false;
					break;
				}
			}

			if (connected)
				++nr;
		}
	}

	return nr;
}

int MonteCarloTreeSearch::GetNrDiagonalRows(const Board& board, const Color4f player, int piecesInARow) const
{
	int nr{ 0 };
	// Check if connected diagonally (top-right to bottom-left)
	for (int row = 0; row < board.GetNrRows() - piecesInARow + 1; row++) {
		for (int col = 0; col < board.GetNrColumns() - piecesInARow + 1; col++)
		{
			bool connected{ true };

			for (int i = 0; i < piecesInARow; i++)
			{
				if (board.GetBoard()[row + i][col + i] != player)
				{
					connected = false;
					break;
				}
			}

			if (connected)
				++nr;
		}
	}

	// Check if connected diagonally (top-right to bottom-left)
	for (int row = 0; row < board.GetNrRows() - piecesInARow + 1; row++) {
		for (int col = piecesInARow - 1; col < board.GetNrColumns(); col++)
		{
			bool connected{ true };

			for (int i = 0; i < piecesInARow; i++)
			{
				if (board.GetBoard()[row + i][col - i] != player)
				{
					connected = false;
					break;
				}
			}

			if (connected)
				++nr;
		}
	}


	return nr;
}

