#pragma once
#include <vector>

class GameState;

struct StateAnalysis
{
	virtual std::vector<int> GetAvailableActions(const GameState& state) const = 0;
	virtual bool CheckWin(const GameState& state, const char& player) const = 0;
	virtual bool CheckDraw(const GameState& state) const = 0;
	virtual bool InProgress(const GameState& state) const = 0;
	virtual float EvaluatePosition(const GameState& state, const char& forPlayer, const char& againstPlayer) const = 0;
};

