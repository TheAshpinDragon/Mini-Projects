// Inspiration: https://www.youtube.com/watch?v=7GgLSnQ48os

#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include <time.h>

#define SCREEN_SIZE olc::vi2d{200, 240}

class BayesianExample : public olc::PixelGameEngine
{
public:
	BayesianExample()
	{
		sAppName = "Bayesian Guessing!";
	}

public:
	olc::vf2d	actualPos{},
				currentGuess{},
				averagePos{};
	olc::vi2d	guessResults{}; // x = # of west results, y = # of north results
	std::vector<olc::vf2d> guesses;
	int			guessCount{0};

	olc::vf2d getRandVec()
	{
		return {
			(abs((rand() % (rand() + 1)) * rand()) % 19999 + 1) / 100.0f,
			(abs((rand() % (rand() + 1)) * rand()) % 19999 + 1) / 100.0f };
	}

	olc::vi2d getGuessResult(olc::vf2d actual, olc::vf2d guess)
	{
		return {guess.x < actual.x, guess.y < actual.y};
	}

	bool OnUserCreate() override
	{
		srand(time(NULL));
		actualPos = getRandVec();
		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{

		if (GetKey(olc::R).bPressed)
		{
			srand(time(NULL));
			actualPos = getRandVec();
			currentGuess = { 0,0 };
			averagePos = { 0,0 };
			guessResults = { 0,0 };
			guessCount = 0;
			guesses.clear();
		}

		if (GetKey(olc::SPACE).bHeld)
		{
			// Make new guess
			currentGuess = getRandVec();
			guesses.push_back(currentGuess);
			guessResults += getGuessResult(actualPos, currentGuess);
			guessCount++;

			// Calculate avg pos
			averagePos = {
				(((guessResults.x) / (guessCount * 1.0f)) * 200.0f),
				(((guessResults.y) / (guessCount * 1.0f)) * 200.0f)
			};

			// Redraw
			Clear(olc::BLACK);
			for(olc::vf2d pos : guesses) DrawCircle(pos, 0, olc::DARK_GREY);
			FillCircle(actualPos, 2, olc::WHITE);
			FillCircle(currentGuess, 2, olc::CYAN);
			FillCircle(averagePos, 1, olc::RED);
			
			// Bottom text: guess proportions
			DrawString(olc::vf2d{0,SCREEN_SIZE.y - 24.0f},
				" North: " + std::to_string(guessResults.y) +
				" South: " + std::to_string(guessCount - guessResults.y) + "\n" +
				" East:  " + std::to_string(guessCount - guessResults.x) +
				" West:  " + std::to_string(guessResults.x) + "\n" + 
				" Total: " + std::to_string(guessCount)
			);

			// Top text: positions
			DrawString(olc::vf2d{ 0,SCREEN_SIZE.y - 40.0f },
				" Act: " + actualPos.strCut(5) + "\n" +
				" Avg: " + averagePos.strCut(5)
			);
		}

		return true;
	}
};

int main()
{
	BayesianExample program;
	if (program.Construct(SCREEN_SIZE.x, SCREEN_SIZE.y, 4, 4, false, false))
		program.Start();
	return 0;
}