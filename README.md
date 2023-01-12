# <div align="center">Monte Carlo Tree Search for Connect 4</div>
## About this project
This is a research project for Gameplay Programming 1 at HowestDAE.
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

	// Find leaf node with maximum win rate
	while (!current_node->IsLeaf())
	{
		MCTSNode* highest_UCD_node{current_node->Children[0]};
		// Find child node that maximises Upper Confidence Boundary
		for (auto& child : current_node->Children)
		{
			if (CalculateUCB(*child) > CalculateUCB(*highest_UCD_node))
				highest_UCD_node = child;
		}

		current_node = highest_UCD_node;
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

