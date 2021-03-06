#include "stdafx.h"

void split(const std::string &str, std::vector<unsigned> &vec);
void readInput(std::vector<unsigned> &rowSums, std::vector<unsigned> &columnSums);

struct node
{
	std::vector<std::vector<bool>> matrix;

	size_t rows;
};

std::vector<unsigned> targetRowSums, targetColumnSums;
size_t rowCount, columnCount;

bool checkIfUnderColumnSums(const node *n)
{
	for (size_t column = 0; column < columnCount; ++column)
	{
		unsigned columnSum = 0;

		for (size_t row = 0; row < n->rows; ++row)
		{
			columnSum += n->matrix[row][column];
		}

		if (columnSum > targetColumnSums[column])
		{
			return false;
		}
	}

	return true;
}

bool checkIfOkay(const node *n)
{
	for (size_t column = 0; column < columnCount; ++column)
	{
		unsigned columnSum = 0;

		for (size_t row = 0; row < n->rows; ++row)
		{
			columnSum += n->matrix[row][column];
		}

		if (columnSum != targetColumnSums[column])
		{
			return false;
		}
	}

	return true;
}

node *solve()
{
	std::vector<node*> expandThese;

	expandThese.push_back(new node());

	do
	{
		node *currentNode;
		currentNode = expandThese[expandThese.size() - 1];
		expandThese.pop_back();

		if (checkIfUnderColumnSums(currentNode))
		{
			if (currentNode->matrix.size() < rowCount)
			{
				std::vector<bool> nextRow(columnCount);
				unsigned rowIndex = currentNode->matrix.size();
				unsigned setOffset = columnCount - targetRowSums[rowIndex];
				std::fill(nextRow.begin(), nextRow.end(), false);
				std::fill(nextRow.begin() + setOffset, nextRow.end(), true);
				do
				{
					node *newNode = new node;
					newNode->matrix = currentNode->matrix;
					newNode->rows = currentNode->rows + 1;
					newNode->matrix.push_back(nextRow);

					expandThese.push_back(newNode);
				} while (std::next_permutation(nextRow.begin(), nextRow.end()));
			}
			else if (checkIfOkay(currentNode))
			{
				for (node *n : expandThese)
				{
					delete n;
				}

				return currentNode;
			}
		}

		delete currentNode;
	} while (!expandThese.empty());

	return nullptr;
}

void printSolution(node *n)
{
	for (size_t row = 0; row < rowCount; ++row)
	{
		for (size_t column = 0; column < columnCount; ++column)
		{
			std::cout << (n->matrix[row][column] ? '1' : '0') << " ";
		}

		std::cout << std::endl;
	}
}

int main()
{
	readInput(targetRowSums, targetColumnSums);
	rowCount = targetRowSums.size();
	columnCount = targetColumnSums.size();

	for (int i = 0; i < targetRowSums.size(); ++i)
	{
		if (targetRowSums[i] > columnCount)
		{
			std::cout << "Invalid input" << std::endl;

			return 0;
		}
	}

	for (int i = 0; i < targetColumnSums.size(); ++i)
	{
		if (targetColumnSums[i] > rowCount)
		{
			std::cout << "Invalid input" << std::endl;

			return 0;
		}
	}

	node *n = solve();

	if (n)
	{
		printSolution(n);
	}
	else
	{
		std::cout << "No solution was found!" << std::endl;
	}

	return 0;
}

void split(const std::string &str, std::vector<unsigned> &vec)
{
	std::istringstream iss(str);
	std::copy(std::istream_iterator<unsigned>(iss),
		std::istream_iterator<unsigned>(),
		std::back_inserter(vec));
}

void readInput(std::vector<unsigned> &rowSums, std::vector<unsigned> &columnSums)
{
	std::string line;

	std::getline(std::cin, line);
	split(line, rowSums);

	std::getline(std::cin, line);
	split(line, columnSums);
}
