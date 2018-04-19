#include "stdafx.h"

void split(const std::string &str, std::vector<unsigned> &vec);
void readInput(std::vector<unsigned> &rowSums, std::vector<unsigned> &columnSums);

struct node
{
	std::vector<std::vector<bool>> matrix;

	size_t rows;
};

std::vector<unsigned> targetRowSums, targetColumnSums;
std::vector<node> solutions;
size_t rowCount, columnCount;

bool checkIfUnderColumnSums(const node &n)
{
	for (size_t column = 0; column < columnCount; ++column)
	{
		unsigned columnSum = 0;

		for (size_t row = 0; row < n.rows; ++row)
		{
			columnSum += n.matrix[row][column];
		}

		if (columnSum > targetColumnSums[column])
		{
			return false;
		}
	}

	return true;
}

bool checkIfOkay(const node &n)
{
	for (size_t column = 0; column < columnCount; ++column)
	{
		unsigned columnSum = 0;

		for (size_t row = 0; row < n.rows; ++row)
		{
			columnSum += n.matrix[row][column];
		}

		if (columnSum != targetColumnSums[column])
		{
			return false;
		}
	}

	return true;
}

void solve()
{
	std::vector<node> expandThese;

	expandThese.push_back(node());

	do
	{
		node currentNode;
		currentNode = expandThese[expandThese.size() - 1];
		expandThese.pop_back();
		
		std::vector<bool> nextRow(columnCount);
		unsigned rowIndex = currentNode.matrix.size();
		unsigned setOffset = columnCount - targetRowSums[rowIndex];
		std::fill(nextRow.begin(), nextRow.end(), false);
		std::fill(nextRow.begin() + setOffset, nextRow.end(), true);

		do
		{
			node newNode = currentNode;
			newNode.rows++;

			newNode.matrix.push_back(nextRow);

			if (checkIfUnderColumnSums(newNode))
			{
				if (newNode.matrix.size() < rowCount)
				{
					expandThese.push_back(newNode);
				}
				else if (checkIfOkay(newNode))
				{
					solutions.push_back(newNode);
				}
			}
		} while (std::next_permutation(nextRow.begin(), nextRow.end()));

	} while (!expandThese.empty());
}

void printSolutions()
{
	std::cout << "Number of solutions " << solutions.size() << std::endl;

	for (const auto &n : solutions)
	{
		for (size_t row = 0; row < rowCount; ++row)
		{
			for (size_t column = 0; column < columnCount; ++column)
			{
				std::cout << (n.matrix[row][column] ? '1' : '0') << " ";
			}

			std::cout << std::endl;
		}

		std::cout << std::endl;
	}
}

int main()
{
	readInput(targetRowSums, targetColumnSums);
	rowCount = targetRowSums.size();
	columnCount = targetColumnSums.size();

	solve();

	printSolutions();

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
