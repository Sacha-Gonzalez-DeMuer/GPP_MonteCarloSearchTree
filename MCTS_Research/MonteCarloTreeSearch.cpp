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
	MCTSNode* promising_node{ };
	for (int i = 0; i < m_NrIterations; i++)
	{
		// Select a node with highest Upper Confidence Boundary
		promising_node = SelectNode(m_RootNode);

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

		// Simulate the game of that move until it finishes (win or draw)
		// Then propagate the result to all the parent nodes
		BackPropagate(node_to_explore, Simulate(node_to_explore));
	}


	// Find node with most visits
	MCTSNode* best_node{ m_RootNode->Children[0]};
	for (const auto& child : m_RootNode->Children)
	{
		if (child->VisitCount > best_node->VisitCount)
			best_node = child;
	}

	delete m_RootNode;
	m_RootNode = nullptr;

	return best_node->State.GetLastMove();
}

MCTSNode* MonteCarloTreeSearch::SelectNode(MCTSNode* fromNode)
{
	//Start
	MCTSNode* current_node{ fromNode };

	if (!current_node)
		return nullptr;

	// Find leaf node
	while (!current_node->IsLeaf())
	{
		MCTSNode* highest_UCB_node{current_node->Children[0]};
		float highest_UCB{ CalculateUCB(*highest_UCB_node) };

		// Find child node that maximises Upper Confidence Boundary
		for (auto& child : current_node->Children)
		{
			float child_UCB{ CalculateUCB(*child) };
			if (child_UCB > highest_UCB)
			{
				highest_UCB = child_UCB;
				highest_UCB_node = child;
			}
		}

		current_node = highest_UCB_node;
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
	// Create a copy to run the simulation on
	GameState state_copy{ node->State };

	// Loop until game ends
	while (true)
	{
		// Play a random move
		const auto available_actions{ m_pStateAnalysis->GetAvailableActions(state_copy) };
		if (!available_actions.empty())
		{
			int rnd_idx{ utils::GetRandomInt(static_cast<int>(available_actions.size())) };
			state_copy.PlacePiece(available_actions[rnd_idx], state_copy.GetCurrentPlayer());
		}
		
		// Check if game is over and return the winner
		if (m_pStateAnalysis->CheckWin(state_copy, state_copy.GetCurrentPlayer()))
			return state_copy.GetCurrentPlayer();

		if (m_pStateAnalysis->CheckWin(state_copy, state_copy.GetWaitingPlayer()))
			return state_copy.GetWaitingPlayer();

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

	// Check if the AI won the simulation
	if (!fromNode->State.IsPlayerTurn(m_pPlayer->GetInitial()) && winningPlayer == m_pPlayer->GetInitial())
		reward = 1;

	// While there is a parent node to visit
	while (current_node != nullptr)
	{
		++current_node->VisitCount;

		// If the AI won the simulation
		if (winningPlayer == m_pPlayer->GetInitial())
			current_node->WinCount += reward;

		current_node = current_node->Parent;

		// If the game ended in a tie, set the reward to 0
		if (winningPlayer == EMPTY)
			reward = 0;
		// Otherwise, switch the reward between 1 and 0
		else
			reward = 1 - reward;
	}
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
