#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

#define SCREEN_SIZE olc::vi2d{256, 240}

class Program : public olc::PixelGameEngine
{
public:
	Program()
	{
		sAppName = "Program";
	}

public:
	bool OnUserCreate() override
	{
		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		return true;
	}
};

int main()
{
	Program program;
	if (program.Construct(SCREEN_SIZE.x, SCREEN_SIZE.y, 4, 4, false, true))
		program.Start();
	return 0;
}