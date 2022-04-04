#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#define SCREEN_WIDTH 256
#define SCREEN_HEIGHT 240

// Override base class with your custom functionality
class Pendulums : public olc::PixelGameEngine
{
public:
	Pendulums()
	{
		// Name your application
		sAppName = "Example";
	}

	struct pendulum {
		pendulum* previous;
		float len = 70, bobMass = 5;
		float ang = 0, angVel = 0, angAcc = 0; // Radians
		olc::vf2d forceVec = { 0, 9.8 * bobMass };

		void update(float deltaTime)
		{
			std::cout << "Acc: " << (cos(ang) * len * forceVec.x) << " + " << (sin(ang) * len * forceVec.y) << " Vel: " << angVel << " Ang: " << ang << std::endl;
			angAcc = (cos(ang) * (1 / (sqrt(len / 9.8) * 2 * 3.14)) * forceVec.y + sin(ang) * (1 / (sqrt(len / 9.8) * 2 * 3.14)) * forceVec.x) / bobMass * 20; //  * 2 * 3.14 * sqrt(len / 9.8)
			angVel += angAcc * deltaTime;
			ang += angVel * deltaTime + 0.5 * angAcc * deltaTime * deltaTime;

			if (previous)
			{
				std::cout << "Previous!" << std::endl;
				previous->forceVec = olc::vf2d{
					forceVec.y - cos(ang) * forceVec.y + 9.8f * bobMass,
					forceVec.x - sin(ang) * forceVec.x,
				};
				previous->update(deltaTime);
			}
		}

		olc::vi2d drawSelf(Pendulums* sim, olc::vi2d origin = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 })
		{
			if (previous)
				origin = previous->drawSelf(sim);

			sim->DrawLine(origin, origin + olc::vi2d(cos(ang) * len, sin(ang) * len));
			sim->FillCircle(origin + olc::vi2d(cos(ang) * len, sin(ang) * len), bobMass);

			return origin + olc::vi2d(cos(ang) * len, sin(ang) * len);
		}
	};

public:
	pendulum P1;
	pendulum P2;

	bool OnUserCreate() override
	{
		// Called once at the start, so create things here

		//P2.previous = &P1;
		P2.len = 100;

		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		// Called once per frame, draws random coloured pixels

		Clear(olc::BLACK);

		P1.update(fElapsedTime);
		P1.drawSelf(this);

		P2.update(fElapsedTime);
		P2.drawSelf(this);

		return true;
	}
};

int main()
{
	Pendulums demo;
	if (demo.Construct(SCREEN_WIDTH, SCREEN_HEIGHT, 4, 4))
		demo.Start();
	return 0;
}