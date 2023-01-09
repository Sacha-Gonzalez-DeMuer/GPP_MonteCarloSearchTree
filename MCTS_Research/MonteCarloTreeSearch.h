#pragma once
#include <array>
#include <memory>
#include "Board.h"
// Forward declarations




//TODO: make class
struct MCTSNode
{
	MCTSNode() {};

	MCTSNode(const Board& state)
		: State(state) {};

	MCTSNode(const MCTSNode& other)
		: WinCount(other.WinCount), VisitCount(other.VisitCount), Children(other.Children), Parent(other.Parent) {};

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

	// State of the game in this node
	Board State;
	UINT VisitCount{ 0 };
	UINT WinCount{ 0 };
	MCTSNode* Parent{nullptr};
	std::vector<MCTSNode*> Children{};

	bool IsLeaf() const { return Children.empty(); }
	void AddChild() { Children.emplace_back(new MCTSNode()); };
};

struct BoardPosition
{
	int row;
	int column;

	friend bool operator==(const BoardPosition& lhs, const BoardPosition& rhs)
	{
		return (lhs.row == rhs.row && lhs.column == rhs.column);
	}

};



class MonteCarloTreeSearch final
{
public:
	MonteCarloTreeSearch();
	~MonteCarloTreeSearch();
	int FindNextMove(const Board& pBoard);


private:

	MCTSNode* m_RootNode;

	MCTSNode* SelectNode(MCTSNode* fromNode);
	void Expand(MCTSNode*& fromNode);
	Color4f Simulate(MCTSNode* node);
	void BackPropagate(MCTSNode* fromNode, Color4f winningPlayer);

	//AI 
	std::vector<int> GetCompletingCellsIndices(const Board& board, const Color4f& color, int piecesInARow) const;
	
	bool IsEmptyWitFullCellBelow(const Board& board, int row, int column) const;
	std::pair<BoardPosition, BoardPosition> GetHorizontalChainStartAndEnd(const Board& board, const Color4f& color, int piecesInARow) const;
	std::pair<BoardPosition, BoardPosition> GetVerticalChainStartAndEnd(const Board& board, const Color4f& color, int piecesInARow) const;
	std::pair<BoardPosition, BoardPosition> GetDiagonalChainStartAndEnd(const Board& board, const Color4f& color, int piecesInARow, bool ascending) const;
	int GetLongestChain(const Board& board, const Color4f player, int& nrOfChains) const;


	float CalculateUCB(const MCTSNode& node) const;
	float EvaluatePosition(const Board& board, const Color4f& forPlayer, const Color4f& againstPlayer) const;
	int GetNrHorizontalChains(const Board& board, const Color4f player, int piecesInARow) const;
	int GetNrVerticalChains(const Board& board, const Color4f player, int piecesInARow) const;
	int GetNrDiagonalChains(const Board& board, const Color4f player, int piecesInARow) const;

	int m_NrIterations{ 100000 };


	//tmp
	Color4f myColor{ PLAYER2 };
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