#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

#define SCREENWIDTH 256
#define SCREENHEIGHT 240
#define SCREENDIM olc::vi2d(SCREENWIDTH, SCREENHEIGHT)
#define FLIP olc::vf2d(1, -1)
#define COMNULL olc::vd2d( std::numeric_limits<float>::max(), std::numeric_limits<float>::max() )

namespace TYPES
{
	const int RECT = 0;
	const int CIRC = 1;
}

// Override base class with your custom functionality
class PhysicsSim : public olc::PixelGameEngine
{
public:
	PhysicsSim()
	{
		// Name your application
		sAppName = "Physics Simulation!";
	}

	struct collider;
	struct object;

	struct pos1d {
		float	pos = 0.0f;
		float	vel = 0.0f;
		float	acc = 0.0f;

		pos1d() = default;

		//pos1d(float _pos, float _vel = 0.0f, float _acc = 0.0f)
		//	: pos(_pos), vel(_vel), acc(_acc) {}

		pos1d(float _pos, float _vel = 0.0f, float _acc = 0.0f)
			: pos{ _pos }, vel{ _vel }, acc{ _acc } {}

		void update(float fElapsedTime)
		{
			float	_vel = vel + acc * fElapsedTime;
			float	_pos = pos + vel * fElapsedTime + .5 * acc * fElapsedTime * fElapsedTime;

			vel = _vel;
			pos = _pos;
		}

		pos1d advance(float fTime)
		{
			float	_acc = acc;
			float	_vel = vel + acc * fTime;
			float	_pos = pos + vel * fTime + .5 * acc * fTime * fTime;

			return pos1d(_pos, _vel, _acc);
		}
		pos1d& operator= (const pos1d& rhs)
		{
			pos = rhs.pos;
			vel = rhs.vel;
			acc = rhs.acc;

			return *this;
		}

		pos1d  operator- (const pos1d& rhs) { return pos1d(pos - rhs.pos, vel - rhs.vel, acc - rhs.acc); }
		pos1d  operator+ (const pos1d& rhs) { return pos1d(pos + rhs.pos, vel + rhs.vel, acc + rhs.acc); }
		
		const std::string to_String() const
		{
			return
				"  POS: " + std::to_string(pos) + " m\n"
				"  VEL: " + std::to_string(vel) + " m/s\n"
				"  ACC: " + std::to_string(acc) + " m/s^2";
		}

		friend std::ostream& operator << (std::ostream& _stream, const pos1d& _out)
		{
			_stream << _out.to_String();
			return _stream;
		}
	};

	struct pos2d {
		olc::vf2d	pos = { 0.0f,0.0f };
		olc::vf2d	vel = { 0.0f,0.0f };
		olc::vf2d	acc = { 0.0f,0.0f };

		pos2d() = default;

		//pos2d(olc::vf2d _pos, olc::vf2d _vel = { 0.0f, 0.0f }, olc::vf2d _acc = { 0.0f, 0.0f })
		//	: pos(_pos), vel(_vel), acc(_acc) {}

		pos2d(olc::vf2d _pos, olc::vf2d _vel = { 0.0f, 0.0f }, olc::vf2d _acc = { 0.0f, 0.0f })
			: pos{ _pos }, vel{ _vel }, acc{ _acc } {}

		void update(float fElapsedTime)
		{
			olc::vf2d	_vel = vel + acc * fElapsedTime;
			olc::vf2d	_pos = pos + vel * fElapsedTime + .5 * acc * fElapsedTime * fElapsedTime;

			vel = _vel;
			pos = _pos;
		}
		
		pos2d advance(float fTime)
		{
			olc::vf2d	_acc = acc;
			olc::vf2d	_vel = vel + acc * fTime;
			olc::vf2d	_pos = pos + vel * fTime + .5 * acc * fTime * fTime;

			return pos2d(_pos, _vel, _acc);
		}

		pos2d flip() { return pos2d(pos * FLIP, vel * FLIP, acc * FLIP); }

		pos2d& operator= (const pos2d& rhs)
		{
			pos = rhs.pos;
			vel = rhs.vel;
			acc = rhs.acc;

			return *this;
		}

		pos2d  operator- (const pos2d& rhs) { return pos2d(pos - rhs.pos, vel - rhs.vel, acc - rhs.acc); }
		pos2d  operator+ (const pos2d& rhs) { return pos2d(pos + rhs.pos, vel + rhs.vel, acc + rhs.acc); }

		const std::string to_String() const
		{
			return
				"  POS: " + pos.str() + " m\n"
				"  VEL: " + vel.str() + " m/s\n"
				"  ACC: " + acc.str() + " m/s^2" ;
		}

		friend std::ostream& operator << (std::ostream& _stream, const pos2d& _out)
		{
			_stream << _out.to_String();
			return _stream;
		}
	};

	struct collider {
		int type;

		// = = = Dimentions/Vertifies = = =
		// std::vector<olc::vi2d> verts;
		olc::vf2d size;

		// = = = Position = = = 
		pos2d		localityBuff;
		pos2d		locality;

		// = = = Rotation = = = 
		pos1d		rotationBuff;
		pos1d		rotation;

		// = = = Properties = = = 
		float		mass;
		olc::vf2d	COM;

		collider() = default;
		collider(int _type = TYPES::RECT, olc::vf2d _size = { 20.0f, 20.0f }, olc::vf2d _pos = { 0.0f, 0.0f }, float _rot = 0.0f, float _mass = 1.0f, olc::vf2d _COM = COMNULL)
			: type(_type), size(_size), locality(_pos, { 0.0f, 0.0f }, { 0.0f, 0.0f }), rotation(_rot, 0.0f, 0.0f), mass(_mass), COM(_COM == COMNULL ? _size / 2.0f : _COM) {}

		virtual void update(float fElapsedTime)
		{
			localityBuff = locality;
			locality.update(fElapsedTime);

			rotationBuff = rotation;
			rotation.update(fElapsedTime);
		}

		// Checks for rectangle collision by default
		virtual bool isColliding(object& _check, float fElapsedTIme)
		{
			olc::vf2d diff =-locality.pos + _check.col.locality.pos;
			olc::vf2d reach = _check.col.size / 2 + size / 2;

			//std::cout << "Diff: \n" << diff << std::endl;

			if (abs(diff.y) < reach.y)
			{
				pos2d delta = _check.col.localityBuff.flip() - localityBuff.flip();
				delta.pos = delta.pos - reach;

				//std::cout << "Self localtiy: \n" << localityBuff << std::endl;
				//std::cout << "other locality:\n" << _check.col.localityBuff << std::endl;
				//std::cout << "diff locality: \n" << delta << std::endl;

				float timeY = 0.0f;
				if (delta.pos.y == 0.0f) // Sitting on a surface
				{
					std::cout << "Im zero!" << std::endl;
				//	locality = localityBuff;
				//	locality.vel.y = 0.0f;
					return true;
				}

				if (delta.acc.y != 0.0f && delta.vel.y != 0.0f) // If there is acceleration and velocity, use quadratic
				{
					// We only care about the positive solution
					olc::vf2d timesY =  solveQuadratic(0.5f * delta.acc.y, delta.vel.y, delta.pos.y);
					
					//std::cout << fElapsedTIme << "  Possible solutions: " << timesY << std::endl;
					timeY = std::max(timesY.x, timesY.y);
				}
				else if (delta.acc.y > 0.0f) // If just acceleration, use sqrt
					timeY = sqrt((delta.pos.y * 2) / delta.acc.y);
				else if(delta.vel.y > 0.0f) // else just divide
					timeY = delta.pos.y / delta.vel.y;
				
				// SOLVE: position they collided

				//std::cout << "Time of collision: quadratic: " << timeY << " liniar: " << abs(delta.pos.y / delta.vel.y) << " TOT: " << fElapsedTIme << std::endl;

				locality = localityBuff.advance(timeY); // abs(delta.pos.y / delta.vel.y)

				//std::cout << "New prev vel: \n" << locality << std::endl;

				locality.vel.y = abs(locality.vel.y) / 2;

				//locality.update(fElapsedTIme - timeY);

				//std::cout << "New final vel: \n" << locality << std::endl << std::endl;

				return true;
			}

			
			return false;
		}

		//olc::vf2d timeOfIntercept() {}

		olc::vf2d solveQuadratic(float a, float b, float c)
		{
			if (a == 0)
				return { 0,0 };

			float root = sqrt((b * b) - 4 * a * c);

			if (isnan(root))
				return { 0,0 };

			return {((-b) + root) / (2 * a), ((-b) - root) / (2 * a) };
		}
	};

	struct object
	{
		// = = = Identity = = = 
		std::string name;

		// = = = Collision = = = 
		collider	col;

		object(std::string _name, olc::vf2d _pos, int _type = TYPES::RECT, olc::vf2d _size = {1.0f, 1.0f}, float _rot = 0.0f, float _mass = 1.0f)
			: name(_name), col(_type, _size, _pos, _rot, _mass) {}

		void update(float fElapsedTime)
		{
			col.update(fElapsedTime);
			// When velocity becomes higher than the framerate, check if there are any objects in the distance between 
		}

		void drawSelf(PhysicsSim* sim)
		{
			sim->FillRect(col.locality.pos * FLIP * sim->zoom + sim->camPos + sim->camOffset, col.size * sim->zoom);
		}
	};
public:

	void drawGrid(float size)
	{
		for (int x = 0; x < SCREENWIDTH / zoom; x += size)
			DrawLine(
				{
					(int)((x + fmod((camPos.x + camOffset.x) / zoom, size)) * zoom),
					0 
				},
				{
					(int)((x + fmod((camPos.x + camOffset.x) / zoom, size)) * zoom),
					SCREENHEIGHT
				},
				olc::GREY);
		
		for (int y = 0; y < SCREENHEIGHT / zoom; y += size)
			DrawLine(
				{
					0,
					(int)((y + fmod((camPos.y + camOffset.y) / zoom, size)) * zoom)
				},
				{
					SCREENWIDTH,
					(int)((y + fmod((camPos.y + camOffset.y) / zoom, size)) * zoom)
				},
				olc::GREY);
	}

public:
	float zoom = 20.0f;
	olc::vf2d camPos = { 0.0f, 0.0f };
	olc::vf2d camOffset = { SCREENWIDTH / 2, SCREENHEIGHT / 2 };

	olc::vf2d dragInit = {0.0f, 0.0f};

	object block =  object("block", olc::vf2d{ 0, 4.00f });
	object ground = object("floor", olc::vf2d{ -50.0f, 0 }, TYPES::RECT, olc::vf2d{ 100.0f, 1.0f });

	bool OnUserCreate() override
	{
		block.col.locality.acc = {0.0f, -9.8f};
		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		Clear(olc::BLACK);

		if (GetMouseWheel())
		{
			zoom += GetMouseWheel() / 60.0f / abs(20 / zoom);
			camPos -= (GetMouseWheel() / 120.0f) * (GetMousePos() - SCREENDIM / 2) / 10.0f;
		}
		else if (GetMouse(0).bPressed) {
			dragInit = GetMousePos();
		}
		else if (GetMouse(0).bHeld)
		{
			camPos += GetMousePos() - dragInit;
			dragInit = GetMousePos();
		}
		else if (GetKey(olc::UP).bPressed)
			block.col.locality.vel.y = 10.0f;

		block.update(fElapsedTime);
		ground.update(fElapsedTime);
		
		block.col.isColliding(ground, fElapsedTime);

		drawGrid(1);
		block.drawSelf(this);
		ground.drawSelf(this);

		return true;
	}
};

int main()
{
	PhysicsSim sim;
	if (sim.Construct(256, 240, 4, 4))
		sim.Start();
	return 0;
}
