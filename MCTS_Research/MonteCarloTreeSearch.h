#pragma once
#include <array>
#include <memory>
#include "Board.h"

struct StateAnalysis;

struct MCTSNode
{
	MCTSNode() {};

	MCTSNode(const GameState& state)
		: State(state) {};

	MCTSNode(const MCTSNode& other)
		: WinCount(other.WinCount), VisitCount(other.VisitCount), Children(other.Children), Parent(other.Parent), State(other.State) {};

	~MCTSNode()
	{
		for (auto& child : Children)
		{
			delete child;
			child = nullptr;
		}
		Children.clear();
	}
	MCTSNode& operator=(const MCTSNode& other)
	{
		VisitCount = other.VisitCount;
		WinCount = other.WinCount;
		State = other.State;
		Children = other.Children;
		return *this;
	}
	MCTSNode& operator=(MCTSNode&& other) = delete;
	MCTSNode(MCTSNode&& other) = delete;

	// State of the game in this node
	GameState State;
	UINT VisitCount{ 0 };
	UINT WinCount{ 0 };
	MCTSNode* Parent{nullptr};
	std::vector<MCTSNode*> Children{};
	bool IsLeaf() const { return Children.empty(); }
};

class MonteCarloTreeSearch final
{
public:
	MonteCarloTreeSearch(Player* player);
	~MonteCarloTreeSearch();
	int FindNextMove(const GameState& pBoard);
private:
	MCTSNode* m_RootNode;

	MCTSNode* SelectNode(MCTSNode* fromNode);
	void Expand(MCTSNode*& fromNode);
	char Simulate(MCTSNode* node);
	void BackPropagate(MCTSNode* fromNode, const char& winningPlayer);

	float CalculateUCB(const MCTSNode& node) const;
	int m_NrIterations{ 10000 };

	Player* m_pPlayer;
	StateAnalysis* m_pStateAnalysis;
};


/*
//Sources
https://youtu.be/UXW2yZndl7U
https://youtu.be/xmImNoDc9Z4
https://medium.com/swlh/tic-tac-toe-at-the-monte-carlo-a5e0394c7bc2
https://pranav-agarwal-2109.medium.com/game-ai-learning-to-play-connect-4-using-monte-carlo-tree-search-f083d7da451e
https://towardsdatascience.com/monte-carlo-tree-search-an-introduction-503d8c04e168
https://www.baeldung.com/java-monte-carlo-tree-search
https://www.sciencedirect.com/science/article/pii/S0304397516302717#:~:text=Firstly%2C%20no%20evaluation%20function%20is,(evaluate)%20these%20different%20positions.
https://softwareengineering.stackexchange.com/questions/263514/why-does-this-evaluation-function-work-in-a-connect-four-game-in-java
*/