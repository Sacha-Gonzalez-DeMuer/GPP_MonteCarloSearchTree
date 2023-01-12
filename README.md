# <div align="center">Monte Carlo Tree Search for Connect 4</div>
## About this project
This is a research project for [Gameplay Programming](https://www.digitalartsandentertainment.be/page/49/Gameplay+programming) at [HowestDAE](https://www.digitalartsandentertainment.be/).
The goal of this project is to better understand how player AI engines like a chess engine work.
However due to the complexity of chess I opted to implement MCTS for Connect 4 instead.

Written in C++ using an SDL framework provided by our lecturers.

## Introduction
MCTS is a tree search algorithm commonly used in game-playing AI. Particularly in games where players need to predict moves that should be taken to win the game such as chess, poker, connect4, etc.

MCTS comprises of two core concepts which is suggested in its name:
* Monte Carlo Simulations
* Trees (In our case specifically Game Trees)

### Monte Carlo Simulations
A Monte Carlo simulation is a statistical method that uses the idea of random sampling together with the law of large numbers to find a numerical solution to a problem that would be otherwise difficult or impossible to solve analytically.

There are 2 main things to take away from this summary which I will go into detail in next:

1. **The Law of Large Numbers:**

	 [The law of large numbers](https://www.investopedia.com/terms/l/lawoflargenumbers.asp#:~:text=What%20Is%20the%20Law%20of,as%20the%20sample%20become%20larger.) states that as the number of trials increases, the average of the results of those trials will converge to the expected value of the underlying random variable. 

	This means that given a large enough sample size, we can come close to the expected real value. 
	For example: to find the true average height of men in a country, we can sample the height of every man in that country, 
	but by measuring the height of a large amount of men we can come *close* to the *true average* height of men in that country.

2. **Random Sampling**

	To feed into the law of large numbers Monte Carlo simulations use random sampling to simulate the behavior of a model.

By combining these two ideas Monte Carlo simulations are able to find an approximation of an optimal result.

In the case of deterministic games like connect 4 the optimal result we're searching for is a win.  
Therefore we can use Monte Carlo simulations to determine wether a move is the optimal move to win by repeatedly simulating a large amount of games after that move until the game finishes.

### Game Trees
However games like these aren't determined by a single move, but rather a sequence of moves where every move creates a new state for the game, branching off into extremely large amounts of different possibilities. 

This presents a problem for Monte Carlo simulations, as we can't run a full simulation for every single possible state.

# Monte Carlo Tree Search
MCTS builds out the game tree in a way where the lines of promising moves are simulated more than moves that are more likely to lose.
This is achieved using a simple algorithm on which MCTS leans, the Upper Confidence Boundary Algorithm.

* **Upper Confidence Boundary Algorithm**
 	
	UCB is used to determine wether a certain state will have a favorable outcome.
	It achieves this by balancing the concepts of "Exploitation" and "Exploration". 
	
	Exploitation is represented by a ratio between the amount of visits at that node to the amount of wins simulated starting from that state.
	
	Exploration makes use of the root node to understand how many times a particular node has been explored compared to the rest of the tree
	it is usually scaled along with an "Exploration factor" which can be adjusted to make the tree lean towards unexplored states.

	Thus the formula for the UCB of a particular node is the following: 
```cpp
float MonteCarloTreeSearch::CalculateUCB(const MCTSNode& node) const
{
	// Avoid division by 0
	if (node.VisitCount == 0)
		return FLT_MAX;

	float UCB{ 0 };

	// Calculate Exploitation
	UCB += static_cast<float>(node.WinCount) / static_cast<float>(node.VisitCount);

	// Calculate Exploration
	UCB += 1.41f * sqrtf(static_cast<float>(m_RootNode->VisitCount) / static_cast<float>(node.VisitCount));

	return UCB;
}
```

Where Root.VisitCount is the total amount of simulations across the tree.

## Stages of MCTS
Now that we understand the core algorithm of MCTS we can explore the steps needed to build and navigate our game tree.
We will need to go through 4 stages in a loop: Selection, Expansion, Simulation and Backpropagation. 

We will loop through these stages until we come to a stop with a limit that we choose.
This can be a time limit or a set number of iterations. I have opted for a fixed amount of iterations to get a better feeling of my trees performance.

### 1. Selection

During selection MCTS will make use of a UCB in order to find a gamestate where the move played has a high chance of winning.
We navigate the tree starting at the root by selecting the child with the highest calculated UCB and continuing to do so until we reach a leaf node
(a node with no more children)
	

```cpp
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
```

### 2. Expansion

If the game of the node that we selected is still in progress (no winner or draw has been reached) we must expand our tree to hold states for every
possible action. So we find every possible move in the selected state and for each move we create a child state where that move is played.
	
	
```cpp
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
```

### 3. Simulation

The simulation stage is where we will run our Monte Carlo simulation. 
We create a copy of the state of our current node and play a game of random moves until the game comes to an end by win or draw.

	
```cpp	
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
```
	
### 4. Backpropagation
When our simulation is done running we propagate the result back up the tree until we reach the root node.
We increment the win and visit counter of every node along the way. Which will be used by UCB on the next iteration.

During backpropagation it is imporant to only increment the win counter on nodes where it is our turn to play, 
since we want to explore nodes that are winning for us and not for the opponent. 
If we were to increment the win counter for the opponent's wins too, 
the algorithm will be inclined to take actions that lead to these opponent's turn states, which are not beneficial to our agent.

```cpp
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

```

### Choosing the best move
Once we've gone through all our iterations the children of our root node, each representing a move to play, will have a large win and visit count.
Choosing which move to play is easy. Since UCB will explore nodes with a higher win chance more often than losing ones, we choose the child with the most visits.

```cpp
// Find node with most visits
MCTSNode* best_node{ m_RootNode->Children[0]};
for (const auto& child : m_RootNode->Children)
{
	if (child->VisitCount > best_node->VisitCount)
		best_node = child;
}

return best_node->State.GetLastMove();
```

## Possibilities for expansion
### Evaluation Function
There are many ways to improve a Monte Carlo Search tree, a popular approach which is also used for well known chess or Go engines is to use an evaluation function.

This evaluation function can be used at different stages of the tree search, however it is imporant to note that this approach can have a negative impact on MCTS, as evaluating a position can be performance intensive. 

During my tests I found that I found that whatever evaluation function I wrote, the payoff in strength was outweight by the loss in performance.
Additionaly, since the evaluation functions I chose to test weren't fully accurate, using an evaluation function resulted in less consitent results regarding the strength of the AI.

But before showing you the different possibilties for a connect 4 evaluation function let me show you where you could use such a function.


* Upper Confidence Boundary
	Since UCB is used to find promising nodes it is only natural that we can incorporate an additional term to our calculation for our evaluation function.
```cpp
float MonteCarloTreeSearch::CalculateUCB(const MCTSNode& node) const
{
	if (node.VisitCount == 0)
		return FLT_MAX;

	float UCB{ 0 };
	// Calculate Exploitation
	UCB += static_cast<float>(node.WinCount) / static_cast<float>(node.VisitCount);

	// Calculate Exploration
	UCB += 2 * sqrtf(static_cast<float>(m_RootNode->VisitCount) / static_cast<float>(node.VisitCount)); 
	
	// Evaluation term
	UCB += EvaluatePosition(node->State, m_pPlayer.GetInitial(), node->State.GetOpponentPiece(m_pPlayer.GetInitial())); // initials are used as pieces internally
	return UCB;
}
```

* Selection stage
	Another approach is to completely replace the UCB function by the evaluation function during selection stage. Since an evaluation stage represents the same idea as UCB, without the need to create and run down a tree. However I wasn't successful in using this approach to make a strong enough AI to consider this approach a success. This is likely due to the inaccuracy of the evaluation functions I used.

* Simulation stage
	We can rework the simulation stage to pick a move which will result in a state with the highest evaluation, rather than choosing a fully random move.
	For this we need to create a deep copy on which to perform every possible action and evaluate the position of the deep copy. 
```cpp
if (!has_moved)
{
	const auto available_actions{ state_copy.GetAvailableActions() };

	float best_evaluation{ 0 };
	int best_move{ available_actions[0] };

	for (const auto& action : available_actions)
	{
		// Create a deep copy to evaluate every action on
		Board deep_copy{ state_copy };
		if (deep_copy.PlacePiece(action, current_player))
		{
			float evaluation{ EvaluatePosition(deep_copy, current_player, opponent_player) };
			if (evaluation > best_evaluation)
			{
				best_evaluation = evaluation;
				best_move = action;
			}
		}
	}
	// Play the move with the best resulting evaluation
	state_copy.PlacePiece(best_move, current_player);
}
```
Although this approach resulted in a relatively strong AI, it still slipped up more than using a pure Monte Carlo approach. Additionaly, the impact on performance was noticable compared to pure randomness.

### Evaluation functions for Connect 4

As mentioned earlier evaluation functions are popular in game AI for complex games like chess and Go where machine learning models are used to evaluate a position. These models have learned from tons of games and can evaluate the position without exact rules in place. 
However it is possible to write an evaluation function by hand.

In the case of connect 4 this can be really simple, and I tested a couple different approaches. Before diving into that let me show you the outline of the evaluation function, the approaches I will be listing all calculate the evaluation of the position.

```cpp
virtual float StateAnalysis::EvaluatePosition(const GameState& state, const char& forPlayer, const char& againstPlayer) const override
{
	// Return max/infinite evaluation for a win
	if (CheckWin(state, forPlayer))
		return FLT_MAX;
		
	// The opposite for a loss
	if (CheckWin(state, againstPlayer))
		return FLT_MIN;
		
	// Equal position or draw = 0
	if (CheckDraw(state))
		return 0;


	float eval{ 0 };
	
	// Perform calculations to evaluate position
	// ...
	
	return eval;
}
```

### Approaches
* **[Evaluation Table](https://softwareengineering.stackexchange.com/q/263514)**
```cpp
static constexpr std::array<std::array<float, 7>, 6> EvalTable
{{
	{3, 4, 5, 7, 5, 4, 3},
	{4, 6, 8, 10, 8, 6, 4},
	{5, 8, 11, 13, 11, 8, 5},
	{5, 8, 11, 13, 11, 8, 5},
	{4, 6, 8, 10, 8, 6, 4},
	{3, 4, 5, 7, 5, 4, 3}
}};
```
This table represents every cell of a connect 4 board. Where its value is the amount of lines that can be made using that cell. A cell with more possibilities for connections inheritely means these cells have a higher value. By checking which player posseses a certain node we can weigh our evaluation. This evaluation function is one of the fastest approaches since it requires relatively little calculations.

```cpp
for (int row = 0; row < state.GetNrRows(); row++) 
{
	for (int col = 0; col < state.GetNrColumns(); col++)
	{
		if (state.GetBoard()[row][col] == forPlayer)
			eval += EvalTable[row][col];
		else if (state.GetBoard()[row][col] == againstPlayer)
			eval -= EvalTable[row][col];
	}
}
```

* **[Longest line](https://softwareengineering.stackexchange.com/a/299446), scaled by amount of lines of this length**
A different approach is to find the longest line for each player, I chose to scale this result by the amount of lines that there are of this length in an attempt to get a more accurate evaluation.
This approach proved to be extremely computationally expensive, as the entire board needs to be scanned for horizontal, vertical and diagonal lines (+ diagonals require checks in two directions) **+** this needs to happen for both players to correctly weigh the evaluation.
Below is the snippet which does the calculation in the evaluation function using analysis methods which can be found in [C4_Analysis.h](https://github.com/SachaGDM/GPP_MonteCarloSearchTree/blob/main/MCTS_Research/C4Analysis.h)

```cpp
int forplayer_nrof_longestchain{ 0 };
int againstplayer_nrof_longestchain{ 0 };
eval += GetLongestChain(state, forPlayer, forplayer_nrof_longestchain) * forplayer_nrof_longestchain;
eval -= GetLongestChain(state, againstPlayer, againstplayer_nrof_longestchain) * againstplayer_nrof_longestchain;
```
* **[Valued chains](https://github.com/prakhar10/Connect4/blob/master/eval_explanation.txt)**
This approach runs into the same problem as finding the longest line, since it goes off of the same principle. The difference here is that we assign a value to each length of a chain, similar to how chess pieces each have their own value. In this example a connect 4 would be 10 points, connect 3 is half of that, connect 2 is half of that.
```cpp
eval += GetNrChains(state, forPlayer, 4) * 10 + GetNrChains(state, forPlayer, 3) * 5 + GetNrChains(state, forPlayer, 2) * 2;
		eval -= GetNrChains(state, againstPlayer, 4) * 10 + GetNrChains(state, againstPlayer, 3) * 5 + GetNrChains(state, againstPlayer, 2) * 2;
```

# Conclusion/Future work
MCTS is a powerful tool to create game-playing AI for both perfect and imperfect information games. For a simple (and solved) game like connect 4 the base implementation of MCTS seems to work optimally, as an evaluation function doesn't give the algorithm any information that it can't get by running a random simulation guided by UCB. However for more complex systems/games like chess, where possibilities are extremely wider I can see evaluation functions greatly improving the performance of the model, both in the computational and competitive aspect.

Originally I had planned on researching the usage of MCTS for chess, after realizing it'd a better step to start small I opted for Connect 4 instead. 
Implementing this has only peaked my interest in the topic. Since I'm an avid chess player I'd love to someday implement my own chess engine and explore the possibilities to improve and optimize a MCTS beyond its basic implementation.

# Sources

* [Monte Carlo Tree Search by John Levine](https://youtu.be/UXW2yZndl7U)
* [Advanced 4. Monte Carlo Tree Search by MIT OpenCourseWare](https://youtu.be/xmImNoDc9Z4)
* [Tic Tac Toe at the Monte Carlo](https://medium.com/swlh/tic-tac-toe-at-the-monte-carlo-a5e0394c7bc2)
* [Game AI: Learning to play Connect 4 using Monte Carlo Tree Search](https://pranav-agarwal-2109.medium.com/game-ai-learning-to-play-connect-4-using-monte-carlo-tree-search-f083d7da451e)
* [Monte Carlo Tree Search: An Introduction](https://towardsdatascience.com/monte-carlo-tree-search-an-introduction-503d8c04e168)
* [Monte Carlo Tree Search for Tic-Tac-Toe Game in Java](https://www.baeldung.com/java-monte-carlo-tree-search)
* [Using evaluation functions in Monte-Carlo Tree Search](https://www.sciencedirect.com/science/article/pii/S0304397516302717#:~:text=Firstly%2C%20no%20evaluation%20function%20is,(evaluate)%20these%20different%20positions.)
* [Why does this evaluation function work in a connect four game in java](https://softwareengineering.stackexchange.com/questions/263514/why-does-this-evaluation-function-work-in-a-connect-four-game-in-java)

# Contact
Gonzalez De Muer Sacha — [gonzalezdemuer.sacha@hotmail.com](mailto:gonzalezdemuer.sacha@hotmail.com)
