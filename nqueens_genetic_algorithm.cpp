#include <iostream>
#include <vector>
#include <stdlib.h>
#include <time.h>
#include <algorithm>
//Chromosome represents one N*N chessboard, where N queens are placed
class Chromosome {
private:
  std::vector <int> board;
  int boardSize;
  int fitness;         //number of clashes on board, fitness = 0 --> solution
  int times_picked;
public:
  Chromosome() {
  }
  Chromosome(int boardSize, int times_picked = 0) {
    this->boardSize = boardSize;
    this->times_picked = times_picked;
    board.reserve(boardSize);
    for (int j = 0; j < boardSize; j++) {
      board.push_back(rand() % (boardSize));
    }
  }
  Chromosome(std::vector <int> board, int boardSize, int times_picked = 0) {
    this->board = board;
    this->times_picked = times_picked;
    this->boardSize = boardSize;
  }
  ~Chromosome() {
  }

  Chromosome &operator=(const Chromosome &other) {
    if (this == &other) {
      return *this;
    }
    this->boardSize = other.boardSize;
    std::copy(&other.board[0], &other.board[0]+boardSize, &board[0]);
    this->fitness = other.fitness;
    return *this;
  }
  void printChromosome() {
    for (int i = 0; i < boardSize; i++) {
      std::cout << board[i] << std::endl;
    }
  }
  std::vector <int> getBoard() {
    return board;
  }

  // This function determines "how good" is an actual board by calculating its fitness (number of clashes)
  // Vertical clashes are not possible due to the data structure chosen
  void calculateChromFitness() {
    //Calculating horizontal clashes by examining the repetitive elements in board
    std::vector <int> repetitive_elements(boardSize, 0);
    for (int i = 0; i < boardSize; i++) {
      repetitive_elements[board[i]]++;
    }
    for (int i = 0; i < boardSize; i++) {
      fitness += repetitive_elements[i] * (repetitive_elements[i] - 1) / 2; //n*(n-1)/2, because of repetition
    }
    // Calculating diagonal clashes based on position indexes
    for (int i = 0; i < boardSize; i++) {
      for (int j = 0; j < boardSize; j++) {
        if (i != j) {
          if (abs(i - j) == abs(board[i] - board[j])) {
            fitness++;
          }
        }
      }
    }
    std::cout << "fitness is:" << fitness<< std::endl;
  }
  //This function merges two Chromosomes (boards), in hope of getting one with a better fitness
  Chromosome *crossover(Chromosome &foreign) {
    int crossover_threshold = rand() % boardSize;
    std::vector <int> child_board(boardSize, 0);
    for (int j = 0; j < boardSize; j++) {
      if (j < crossover_threshold) {
        child_board[j] = board[j];
      } else {
        child_board[j] = foreign.board[j];
      }
    }
    return new Chromosome(child_board, boardSize);
  }
  //This function changes one index in board forcefully with a certain probability
  void mutate(double mutationProbability) {
    if (mutationProbability * 100 > rand() % 100) {
      board[rand() % boardSize] = rand() % boardSize;
    }
  }
  bool operator>(const Chromosome &other) {
    return this->fitness > other.fitness;
  }
  bool operator<(const Chromosome &other) {
    return this->fitness < other.fitness;
  }
  int getFitness() const {
    return fitness;
  }
};

struct compFitness {
  inline bool operator () (const Chromosome& a, const Chromosome& b ) {
    return (a.getFitness() < b.getFitness());
  }
};
// Population represents a set of boards on which we iterate the genetic operations during each generation
class Population {
private:
  std::vector <Chromosome> population;
  int populationSize;
  int generation;
public:
  Population(int boardSize, int populationSize, int generation = 1) {
    this->generation = generation;
    this->populationSize = populationSize;
    population.reserve(populationSize * sizeof(Chromosome));
    for (int i = 0; i < populationSize; i++) {
      population.push_back(Chromosome(boardSize));
    }
  }

  Population(std::vector <Chromosome> population) {
    this->population = population;
    this->populationSize = population.size();
  }

  ~Population() {

  }
//This function calculates the fitness of each Chromosome in the population
  void calculatePopFitness() {
    for (int i = 0; i < population.size(); i++) {
      population[i].calculateChromFitness();
    }
  }
//This function sorts the population by fitness (best fitness gets [0] index)
  void sortPopulation() {
    std::sort(population.begin(), population.end(), compFitness());
  }
  std::vector <Chromosome> getPopulation() {
    return population;
  }
  void printPopulation() {
    for (int i = 0; i < populationSize; i++) {
      population[i].printChromosome();
    }
  }
  int getPopsize() {
    return populationSize;
  }
  void printWinner() {
    for (int i = 0; i < population[0].getBoard().size(); i++) {
      for (int j = 0; j < population[0].getBoard().size(); j++) {
        if (population[0].getBoard()[i] == j) {
          std::cout<<"1"<<" ";
        } else {
          std::cout<<"0"<<" ";
        }
      }
      std::cout<<std::endl;
    }
  }
};
//Selecting Chromosomes from population for genetic operations
//It selects from the best Chromosomes (selection_threshold) with a certain probability (topSelectProb)
int select(Population p, int selection_threshold, int topSelectProb) {
  if (rand() % 100 < topSelectProb * 100) {
    return rand() % selection_threshold;
  } else {
    return rand() % (p.getPopsize() - selection_threshold);
  }
}
//This function collects all operations and completes a generation, returning a new population
Population doGeneration(Population currentPop, int boardSize, int populationSize, double topFitnessPerc, double topSelectProb,
  double mutationProbability) {
    currentPop.calculatePopFitness();
    currentPop.sortPopulation();
    if (currentPop.getPopulation()[0].getFitness() == 0) {
      std::cout << "i have a solution" << std::endl;
      currentPop.printWinner();
      return currentPop;
    }
    int selection_threshold = (int)(currentPop.getPopulation().size() * topFitnessPerc);
    int i = 0;
    int index1;
    int index2;
    Chromosome* newChrom;
    std::vector <Chromosome> newPopulation;
    newPopulation.reserve(currentPop.getPopulation().size());
    while (i < currentPop.getPopulation().size()) {
      index1 = select(currentPop, selection_threshold, topSelectProb);
      index2 = select(currentPop, selection_threshold, topSelectProb);
      while (index1 == index2) {
        index1 = select(currentPop, selection_threshold, topSelectProb);
      }
      newChrom = currentPop.getPopulation()[index1].crossover(currentPop.getPopulation()[index2]);
      newChrom->mutate(mutationProbability);
      newPopulation.push_back(*newChrom);
      i++;
    }
    Population newPop(newPopulation);
    std::cout << "Generation is done" << std::endl;
    return doGeneration(newPop, boardSize, populationSize, topFitnessPerc, topSelectProb, mutationProbability);
  }
  int main() {
    srand(time(NULL));
    Population p(10, 40);
    doGeneration(p, 10, 40, 0.3, 0.7, 0.3);
    return 0;
  }
