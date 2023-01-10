#pragma once
#include "pch.h"
#include "MonteCarloTreeSearch.h"
#include <random>
#include <iostream>			
#include "Player.h"
#include "C4Analysis.h"

MonteCarloTreeSearch::MonteCarloTreeSearch(Player* player)
	: m_RootNode{ new MCTSNode() }
	, m_pPlayer{player}
	, m_pStateAnalysis{ new C4_Analysis() }
{
}

MonteCarloTreeSearch::~MonteCarloTreeSearch()
{
	if (m_RootNode)
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
	
}


int MonteCarloTreeSearch::FindNextMove(const GameState& pBoard)
{
	m_RootNode = new MCTSNode(pBoard);
	MCTSNode* root{ m_RootNode };
	MCTSNode* promising_node{ root };
	for (int i = 0; i < m_NrIterations; i++)
	{
		// Select a node with highest Upper Confidence Boundary
		promising_node = SelectNode(root);

		// If state of node isn't complete, make a new child node for all possible moves
		if (m_pStateAnalysis->InProgress(promising_node->State))
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
	if (best_node)
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
	const auto& available_actions{ m_pStateAnalysis->GetAvailableActions(fromNode->State) };

	std::vector<MCTSNode*> new_children{};
	for (const auto& action : available_actions)
	{
		// Make a copy of the board state
		GameState new_state{ fromNode->State };

		// Play the available action
		new_state.PlacePiece(action, new_state.IsPlayer1Turn() 
			? new_state.GetP1Piece() : new_state.GetP2Piece());

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
char MonteCarloTreeSearch::Simulate(MCTSNode* node)
{
	char current_player{ };
	char opponent_player{ };
	GameState state_copy{ node->State };

	// Loop forever
	while (true)
	{
		if (state_copy.IsPlayer1Turn())
		{
			current_player = state_copy.GetP1Piece();
			opponent_player = state_copy.GetP2Piece();
		}
		else
		{
			current_player = state_copy.GetP2Piece();
			opponent_player = state_copy.GetP1Piece();
		}

		bool has_moved{ false };
		int rnd_move_idx{ -1 };

		// Play random move
		if (!has_moved)
		{
			const auto available_actions{ m_pStateAnalysis->GetAvailableActions(state_copy) };

			if (available_actions.size() == 0)
			{

			}
			else
			{
				int rnd_idx{ utils::GetRandomInt(static_cast<int>(available_actions.size())) };
				state_copy.PlacePiece(available_actions[rnd_idx], current_player);
			}
			
		}


		if (m_pStateAnalysis->CheckWin(state_copy, state_copy.GetP1Piece()))
			return state_copy.GetP1Piece();

		if (m_pStateAnalysis->CheckWin(state_copy, state_copy.GetP2Piece()))
			return state_copy.GetP2Piece();

		if (m_pStateAnalysis->CheckDraw(state_copy))
			return EMPTY;
	}
}


/*
 Once the algorithm reaches the end of the game,
 it evaluates the state to figure out which player has won.
 It traverses upwards to the root and increments visit score for all visited nodes.
 It also updates win score for each node if the player for that position has won the playout.
*/
void MonteCarloTreeSearch::BackPropagate(MCTSNode* fromNode, const char& winningPlayer)
{
	MCTSNode* current_node{ fromNode };
	int reward{ 0 };

	// Check if I won the simulation
	if (fromNode->State.IsPlayerTurn(m_pPlayer->GetInitial()) && winningPlayer == m_pPlayer->GetInitial())
		reward = 1;


	while (current_node != nullptr)
	{
		++current_node->VisitCount;

		if (winningPlayer == m_pPlayer->GetInitial())
			current_node->WinCount += reward;

		current_node = current_node->Parent;

		if (winningPlayer == EMPTY)
			reward = 0;
		else
			reward = 1 - reward;
	};
}

MCTSNode* MonteCarloTreeSearch::GetChildWhereMoveWasPlayed(MCTSNode* node, int move) const
{
	for (const auto& child : node->Children)
	{
		if (child->State.GetLastMove() == move)
			return child;
	}

	return nullptr;
}



float MonteCarloTreeSearch::CalculateUCB(const MCTSNode& node) const
{
	if (node.VisitCount == 0)
		return FLT_MAX;

	float UCB{ 0 };

	// Calculate Exploitation
	UCB += static_cast<float>(node.WinCount) / static_cast<float>(node.VisitCount);

	// Calculate Exploration
	UCB += 1.41f * sqrtf(static_cast<float>(m_RootNode->VisitCount) / static_cast<float>(node.VisitCount));

	return UCB;
}
