#pragma once
#include "StateAnalysis.h"

struct C4_Analysis final : public StateAnalysis
{
	static constexpr std::array<std::array<float, 7>, 6> EvalTable{ {
	{3, 4, 5, 7, 5, 4, 3},
	{4, 6, 8, 10, 8, 6, 4},
	{5, 8, 11, 13, 11, 8, 5},
	{5, 8, 11, 13, 11, 8, 5},
	{4, 6, 8, 10, 8, 6, 4},
	{3, 4, 5, 7, 5, 4, 3}}
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

	enum class Orientation { Horizontal, Vertical, Ascending, Descending };
	struct Chain
	{
		BoardPosition start;
		BoardPosition end;
		Orientation orientation;
	};

	virtual bool StateAnalysis::InProgress(const GameState& state) const override
	{
		return state.GetNrPieces() < state.GetNrColumns() * state.GetNrRows();
	}


	virtual std::vector<int> StateAnalysis::GetAvailableActions(const GameState& state) const override
	{
		std::vector<int> availableActions{};

		for (int col = 0; col < state.GetNrColumns(); ++col)
		{
			// Check if the column is empty.
			if (state.GetBoard()[state.GetNrRows() - 1][col] == EMPTY)
			{
				availableActions.push_back(col);
			}
		}

		return availableActions;
	}


	bool CheckPiecesInAHorizontalRow(const GameState& state, const char& player, int piecesInARow) const
	{
		// Check if connected horizontally
		for (int row = 0; row < state.GetNrRows(); row++) {
			for (int col = 0; col < state.GetNrColumns() - piecesInARow + 1; col++)
			{
				bool connected{ true };
				for (int i = 0; i < piecesInARow; i++) {
					if (state.GetBoard()[row][col + i] != player) {
						connected = false;
						break;
					}
				}

				if (connected)
					return true;
			}
		}

		return false;
	}


	bool CheckPiecesInAVerticalRow(const GameState& state, const char& player, int piecesInARow) const
	{
		// Check if connected vertically
		for (int row = 0; row < state.GetNrRows() - piecesInARow + 1; row++) {
			for (int col = 0; col < state.GetNrColumns(); col++)
			{
				bool connected{ true };
				for (int i = 0; i < piecesInARow; i++)
				{
					if (state.GetBoard()[row + i][col] != player)
					{
						connected = false;
						break;
					}
				}

				if (connected)
					return true;
			}
		}

		return false;
	}



	bool CheckPiecesInADiagonalRow(const GameState& state, const char& player, int piecesInARow, bool ascending) const
	{
		if (ascending)
		{
			// Check if connected diagonally (top-right to bottom-left)
			for (int row = 0; row < state.GetNrRows() - piecesInARow + 1; row++) {
				for (int col = 0; col < state.GetNrColumns() - piecesInARow + 1; col++)
				{
					bool connected{ true };

					for (int i = 0; i < piecesInARow; i++)
					{
						if (state.GetBoard()[row + i][col + i] != player)
						{
							connected = false;
							break;
						}
					}

					if (connected)
						return true;
				}
			}
		}
		else
		{
			// Check if connected diagonally (top-right to bottom-left)
			for (int row = 0; row < state.GetNrRows() - piecesInARow + 1; row++) {
				for (int col = piecesInARow - 1; col < state.GetNrColumns(); col++)
				{
					bool connected{ true };

					for (int i = 0; i < piecesInARow; i++)
					{
						if (state.GetBoard()[row + i][col - i] != player)
						{
							connected = false;
							break;
						}
					}

					if (connected)
						return true;
				}
			}
		}
		return false;
	}


	bool CheckPiecesInARow(const GameState& state, const char& player, int piecesInARow) const
	{
		// Check if connected horizontally
		if (CheckPiecesInAHorizontalRow(state, player, piecesInARow))
			return true;

		if (CheckPiecesInAVerticalRow(state, player, piecesInARow))
			return true;

		if (CheckPiecesInADiagonalRow(state, player, piecesInARow, false))
			return true;

		if (CheckPiecesInADiagonalRow(state, player, piecesInARow, true))
			return true;

		return false;
	}

	virtual bool StateAnalysis::CheckWin(const GameState& state, const char& player) const override
	{
		return CheckPiecesInARow(state, player, 4);
	}

	virtual bool StateAnalysis::CheckDraw(const GameState& state) const override
	{
		return state.GetNrPieces() == state.GetNrColumns() * state.GetNrRows();
	}



	bool IsEmptyWitFullCellBelow(const GameState& state, int row, int column) const
	{
		//Check if cell is empty
		if (state.GetBoard()[row][column] != EMPTY) {
			return false;
		}

		//Check if cell below is full
		if (row - 1 < 0 || state.GetBoard()[row - 1][column] != EMPTY)
			return true;

		return false;
	}




	std::pair<BoardPosition, BoardPosition> GetHorizontalChainStartAndEnd(const GameState& state, const char& player, int piecesInARow) const
	{
		BoardPosition startPos{ -1, -1 };
		BoardPosition endPos{ -1,-1 };

		// Check if connected horizontally
		for (int row = 0; row < state.GetNrRows(); row++) {
			for (int col = 0; col < state.GetNrColumns() - piecesInARow + 1; col++)
			{
				bool connected{ true };
				startPos.row = row;
				startPos.column = col;

				for (int i = 0; i < piecesInARow; i++)
				{
					if (state.GetBoard()[row][col + i] != player)
					{
						connected = false;
						break;
					}
				}

				if (connected)
				{
					endPos.row = row;
					endPos.column = col + piecesInARow - 1;
					return std::pair<BoardPosition, BoardPosition>(startPos, endPos);
				}
			}
		}

		return std::pair<BoardPosition, BoardPosition>(INVALID_BOARD_POSITION, INVALID_BOARD_POSITION);
	}

	std::pair<BoardPosition, BoardPosition> GetVerticalChainStartAndEnd(const GameState& state, const char& player, int piecesInARow) const
	{
		BoardPosition startPos{ -1, -1 };
		BoardPosition endPos{ -1,-1 };

		// Check if connected vertically
		for (int row = 0; row < state.GetNrRows() - piecesInARow; row++) {
			for (int col = 0; col < state.GetNrColumns(); col++)
			{
				bool connected{ true };
				startPos.row = row;
				startPos.column = col;

				for (int i = 0; i < piecesInARow; i++)
				{
					if (state.GetBoard()[row + i][col] != player) {
						connected = false;
						break;
					}
				}

				if (connected)
				{
					endPos.row = row + piecesInARow - 1;
					endPos.column = col;

					return std::pair<BoardPosition, BoardPosition>(startPos, endPos);
				}
			}
		}

		return std::pair<BoardPosition, BoardPosition>(INVALID_BOARD_POSITION, INVALID_BOARD_POSITION);
	}

	std::pair<BoardPosition, BoardPosition> GetDiagonalChainStartAndEnd(const GameState& state, const char& player, int piecesInARow, bool ascending) const
	{
		BoardPosition startPos{ -1, -1 };
		BoardPosition endPos{ -1,-1 };

		if (ascending)
		{
			// Check if connected diagonally (top-right to bottom-left)
			for (int row = 0; row < state.GetNrRows() - piecesInARow + 1; row++) {
				for (int col = 0; col < state.GetNrColumns() - piecesInARow + 1; col++)
				{
					bool connected{ true };
					startPos.row = row;
					startPos.column = col;

					for (int i = 0; i < piecesInARow; i++)
					{
						if (state.GetBoard()[row + i][col + i] != player)
						{
							connected = false;
							break;
						}
					}

					if (connected)
					{
						endPos.row = row + piecesInARow - 1;
						endPos.column = col + piecesInARow - 1;
						return std::pair<BoardPosition, BoardPosition>(startPos, endPos);
					}
				}
			}
		}
		else
		{
			// Check if connected diagonally (top-right to bottom-left)
			for (int row = 0; row < state.GetNrRows() - piecesInARow + 1; row++) {
				for (int col = piecesInARow - 1; col < state.GetNrColumns(); col++)
				{
					bool connected{ true };
					startPos.row = row;
					startPos.column = col;

					for (int i = 0; i < piecesInARow; i++)
					{
						if (state.GetBoard()[row + i][col - i] != player)
						{
							connected = false;
							break;
						}
					}

					if (connected)
					{
						endPos.row = row + piecesInARow - 1;
						endPos.column = col - piecesInARow + 1;
						return std::pair<BoardPosition, BoardPosition>(startPos, endPos);
					}
				}
			}
		}

		return std::pair<BoardPosition, BoardPosition>(INVALID_BOARD_POSITION, INVALID_BOARD_POSITION);
	}




	int GetNrHorizontalChains(const GameState& state, const char player, int piecesInARow) const
	{
		int nr{ 0 };
		// Check if connected horizontally
		for (int row = 0; row < state.GetNrRows(); row++) {
			for (int col = 0; col < state.GetNrColumns() - piecesInARow + 1; col++)
			{
				bool connected{ true };
				for (int i = 0; i < piecesInARow; i++) {
					if (state.GetBoard()[row][col + i] != player) {
						connected = false;
						break;
					}
				}

				if (connected)
				{
					++nr;
					break;
				}
			}
		}

		return nr;
	}

	int GetNrVerticalChains(const GameState& state, const char player, int piecesInARow) const
	{
		int nr{ 0 };

		// Check if connected vertically
		for (int row = 0; row < state.GetNrRows() - piecesInARow + 1; row++) {
			for (int col = 0; col < state.GetNrColumns(); col++)
			{
				bool connected{ true };
				for (int i = 0; i < piecesInARow; i++)
				{
					if (state.GetBoard()[row + i][col] != player)
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

	int GetNrDiagonalChains(const GameState& state, const char player, int piecesInARow) const
	{
		int nr{ 0 };
		// Check if connected diagonally (top-right to bottom-left)
		for (int row = 0; row < state.GetNrRows() - piecesInARow + 1; row++) {
			for (int col = 0; col < state.GetNrColumns() - piecesInARow + 1; col++)
			{
				bool connected{ true };

				for (int i = 0; i < piecesInARow; i++)
				{
					if (state.GetBoard()[row + i][col + i] != player)
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
		for (int row = 0; row < state.GetNrRows() - piecesInARow + 1; row++) {
			for (int col = piecesInARow - 1; col < state.GetNrColumns(); col++)
			{
				bool connected{ true };

				for (int i = 0; i < piecesInARow; i++)
				{
					if (state.GetBoard()[row + i][col - i] != player)
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

	int GetNrChains(const GameState& state, const char player, int piecesInARow) const
	{
		return
			GetNrHorizontalChains(state, player, piecesInARow)
			+ GetNrVerticalChains(state, player, piecesInARow)
			+ GetNrDiagonalChains(state, player, piecesInARow);
	}





	std::vector<int> GetHorizontalCompletingCellsIndices(const GameState& state, const char& player, int piecesInARow) const
	{
		std::vector<int> completingColumnsIndices;

		// Check if theres a chain of piecesInARow - 1 (almost complete chain) 
		if (CheckPiecesInAHorizontalRow(state, player, piecesInARow - 1))
		{
			// Get the almost completed row
			std::pair<BoardPosition, BoardPosition> horizontalRow{ GetHorizontalChainStartAndEnd(state, player, piecesInARow - 1) };

			if (horizontalRow.first != INVALID_BOARD_POSITION || horizontalRow.second != INVALID_BOARD_POSITION)
			{
				// Check if you can place a piece in the positions next to it
				if (horizontalRow.first.column - 1 >= 0
					&& IsEmptyWitFullCellBelow(state, horizontalRow.first.row, horizontalRow.first.column - 1)) //check left side
				{
					completingColumnsIndices.push_back(horizontalRow.first.column - 1); //save that move

				}

				if (horizontalRow.second.column + 1 < state.GetNrColumns()
					&& IsEmptyWitFullCellBelow(state, horizontalRow.second.row, horizontalRow.second.column + 1)) //check right side
				{
					completingColumnsIndices.push_back(horizontalRow.second.column + 1);

				}
			}
		}

		return completingColumnsIndices;
	}

	std::vector<int> GetVerticalCompletingCellsIndices(const GameState& state, const char& player, int piecesInARow) const
	{
		std::vector<int> completingColumnsIndices;

		if (CheckPiecesInAVerticalRow(state, player, piecesInARow - 1))
		{
			// Get the almost completed row
			std::pair<BoardPosition, BoardPosition> verticalRow{ GetVerticalChainStartAndEnd(state, player, piecesInARow - 1) };

			if (verticalRow.first != INVALID_BOARD_POSITION || verticalRow.second != INVALID_BOARD_POSITION)
			{
				// Check if you can place a piece on top of it
				if (verticalRow.second.row + 1 < state.GetNrRows()
					&& IsEmptyWitFullCellBelow(state, verticalRow.second.row + 1, verticalRow.second.column))  //check tower column
				{
					completingColumnsIndices.push_back(verticalRow.second.column); //save that move
				}
			}
		}

		return completingColumnsIndices;
	}

	std::vector<int> GetDiagonalCompletingCellsIndices(const GameState& state, const char& player, int piecesInARow) const
	{
		std::vector<int> completingColumnsIndices;


		if (CheckPiecesInADiagonalRow(state, player, piecesInARow - 1, true))
		{
			std::pair<BoardPosition, BoardPosition> diagonalRow{ GetDiagonalChainStartAndEnd(state, player, piecesInARow - 1, true) };

			if (diagonalRow.first != INVALID_BOARD_POSITION || diagonalRow.second != INVALID_BOARD_POSITION)
			{
				// Check if you can place a piece in the positions next to it
				if (diagonalRow.first.column - 1 >= 0 && diagonalRow.first.row - 1 >= 0
					&& IsEmptyWitFullCellBelow(state, diagonalRow.first.row - 1, diagonalRow.first.column - 1))
				{
					completingColumnsIndices.push_back(diagonalRow.first.column - 1);
				}

				if (diagonalRow.second.column + 1 < state.GetNrColumns() && diagonalRow.second.row + 1 < state.GetNrRows()
					&& IsEmptyWitFullCellBelow(state, diagonalRow.second.row + 1, diagonalRow.second.column + 1))
				{
					completingColumnsIndices.push_back(diagonalRow.first.column + 1);
				}
			}
		}

		if (CheckPiecesInADiagonalRow(state, player, piecesInARow - 1, false))
		{
			std::pair<BoardPosition, BoardPosition> diagonalRow{ GetDiagonalChainStartAndEnd(state, player, piecesInARow - 1, false) };

			if (diagonalRow.first != INVALID_BOARD_POSITION || diagonalRow.second != INVALID_BOARD_POSITION)
			{
				// Check if you can place a piece in the positions next to it
				if (diagonalRow.first.column + 1 < state.GetNrColumns() && diagonalRow.first.row - 1 >= 0
					&& IsEmptyWitFullCellBelow(state, diagonalRow.first.row - 1, diagonalRow.first.column + 1))
				{
					completingColumnsIndices.push_back(diagonalRow.first.column - 1);
				}

				if (diagonalRow.second.column - 1 > 0 && diagonalRow.second.row + 1 < state.GetNrRows()
					&& IsEmptyWitFullCellBelow(state, diagonalRow.second.row + 1, diagonalRow.second.column - 1))
				{
					completingColumnsIndices.push_back(diagonalRow.second.column + 1);
				}
			}
		}


		return completingColumnsIndices;
	}

	bool IsDoubleOpenChain(const GameState& state, const Chain& chain) const
	{
		switch (chain.orientation)
		{
		case Orientation::Horizontal:
			// Check if you can place a piece in the positions next to it
			if (chain.start.column - 1 >= 0
				&& IsEmptyWitFullCellBelow(state, chain.start.row, chain.start.column - 1)) //check left side
			{
				if (chain.end.column + 1 < state.GetNrColumns()
					&& IsEmptyWitFullCellBelow(state, chain.end.row, chain.end.column + 1)) //check right side
				{
					return true;
				}
			}

			break;

		case Orientation::Vertical:
			// Check if you can place a piece on top of it
			if (chain.end.row + 1 < state.GetNrRows()
				&& IsEmptyWitFullCellBelow(state, chain.end.row + 1, chain.end.column))
			{
				return true;
			}
			break;

		case Orientation::Ascending:
			// Check if you can place a piece in the positions next to it
			if (chain.start.column - 1 >= 0 && chain.start.row - 1 >= 0
				&& IsEmptyWitFullCellBelow(state, chain.start.row - 1, chain.start.column - 1))
			{
				if (chain.end.column + 1 < state.GetNrColumns() && chain.end.row + 1 < state.GetNrRows()
					&& IsEmptyWitFullCellBelow(state, chain.end.row + 1, chain.end.column + 1))
				{
					return true;
				}
			}
			break;

		case Orientation::Descending:

			// Check if you can place a piece in the positions next to it
			if (chain.start.column + 1 < state.GetNrColumns() && chain.start.row - 1 >= 0
				&& IsEmptyWitFullCellBelow(state, chain.start.row - 1, chain.start.column + 1))
			{
				if (chain.end.column - 1 > 0 && chain.end.row + 1 < state.GetNrRows()
					&& IsEmptyWitFullCellBelow(state, chain.end.row + 1, chain.end.column - 1))
				{
					return true;
				}
			}

			break;
		}

		return false;
	}




	std::vector<int> GetCompletingCellsIndices(const GameState& state, const char& player, int piecesInARow) const
	{
		std::vector<int> completingColumnsIndices{};

		const auto horizontalIndices{ GetHorizontalCompletingCellsIndices(state, player, piecesInARow) };

		const auto verticalIndices{ GetVerticalCompletingCellsIndices(state, player, piecesInARow) };

		const auto diagonalIndices{ GetDiagonalCompletingCellsIndices(state, player, piecesInARow) };

		completingColumnsIndices.insert(completingColumnsIndices.end(), horizontalIndices.begin(), horizontalIndices.end());
		completingColumnsIndices.insert(completingColumnsIndices.end(), verticalIndices.begin(), verticalIndices.end());
		completingColumnsIndices.insert(completingColumnsIndices.end(), diagonalIndices.begin(), diagonalIndices.end());

		return completingColumnsIndices;
	}

	int GetLongestChain(const GameState& state, const char player, int& nrOfChains) const
	{
		// Find the longest chain each player has, and the amount of chains of that length they have
		int longest_chain{ 0 };

		nrOfChains = 0;

		// Check longest horizontal chain
		int highest_nr_horizontal_chains{ 0 };
		for (int i{ 0 }; i < 4; ++i)
		{
			int nr_horizontal_chains{ GetNrHorizontalChains(state, player, i) };

			if (nr_horizontal_chains > 0)
			{
				longest_chain = i;
				highest_nr_horizontal_chains = nr_horizontal_chains;
			}
			else
			{
				nrOfChains += highest_nr_horizontal_chains;
			}
		}

		// Check if any vertical chain is bigger
		int highest_nr_vertical_chains{ 0 };
		for (int i{ longest_chain }; i < 4; ++i)
		{
			int nr_vertical_chains{ GetNrVerticalChains(state, player, i) };

			if (nr_vertical_chains > 0)
			{
				longest_chain = i;
				highest_nr_vertical_chains = nr_vertical_chains;
			}
			else
			{
				// If chain is longer than previous, reset nr of chains, otherside add the amount of chains
				if (i > longest_chain)
					nrOfChains = highest_nr_vertical_chains;
				else
					nrOfChains += highest_nr_vertical_chains;
			}
		}

		// Check if any diagonal chain is bigger
		int highest_nr_diagonal_chains{ 0 };
		for (int i{ longest_chain }; i < 4; ++i)
		{
			int nr_diagonal_chains{ GetNrDiagonalChains(state, player, i) };

			if (nr_diagonal_chains > 0)
			{
				longest_chain = i;
				highest_nr_diagonal_chains = nr_diagonal_chains;
			}
			else
			{
				if (i > longest_chain)
					nrOfChains = highest_nr_diagonal_chains;
				else
					nrOfChains += highest_nr_diagonal_chains;
			}
		}

		return longest_chain;
	}



	virtual float StateAnalysis::EvaluatePosition(const GameState& state, const char& forPlayer, const char& againstPlayer) const override
	{
		if (CheckWin(state, forPlayer))
			return FLT_MAX;

		if (CheckWin(state, againstPlayer))
			return FLT_MIN;

		if (CheckDraw(state))
			return 0;


		float eval{ 0 };
		// https://github.com/prakhar10/Connect4/blob/master/eval_explanation.txt
		eval += GetNrChains(state, forPlayer, 4) * 10 + GetNrChains(state, forPlayer, 3) * 5 + GetNrChains(state, forPlayer, 2) * 2;
		eval -= GetNrChains(state, againstPlayer, 4) * 10 + GetNrChains(state, againstPlayer, 3) * 5 + GetNrChains(state, againstPlayer, 2) * 2;

		// https://softwareengineering.stackexchange.com/a/299446
		int forplayer_nrof_longestchain{ 0 };
		int againstplayer_nrof_longestchain{ 0 };
		eval += GetLongestChain(state, forPlayer, forplayer_nrof_longestchain) * 5;
		eval -= GetLongestChain(state, againstPlayer, againstplayer_nrof_longestchain) * 5;



		for (int row = 0; row < state.GetNrRows(); row++) {
			for (int col = 0; col < state.GetNrColumns(); col++)
			{
				if (state.GetBoard()[row][col] == forPlayer)
					eval += EvalTable[row][col];
				else if (state.GetBoard()[row][col] == againstPlayer)
					eval -= EvalTable[row][col];
			}
		}

		return eval;
	}
};