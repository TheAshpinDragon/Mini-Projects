#include "olcPixelGameEngine.h"

#define SCREENWIDTH 512
#define SCREENHEIGHT 480
#define SCREENDIM olc::vi2d(SCREENWIDTH, SCREENHEIGHT)
#define ACCURACY 0.000001f
#define DEFAULTZOOM 40.0f
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

		// = = = = FUNCTIONS = = = = //

		void set(pos2d set) 
		{
			acc = set.acc;
			vel = set.vel;
			pos = set.pos;
		}

		void setX(pos2d set)
		{
			acc.x = set.acc.x;
			vel.x = set.vel.x;
			pos.x = set.pos.x;
		}

		void setY(pos2d set)
		{
			acc.y = set.acc.y;
			vel.y = set.vel.y;
			pos.y = set.pos.y;
		}

		// Update X & Y of self
		void update(float fElapsedTime)
		{
			olc::vf2d	_vel = vel + acc * fElapsedTime;
			olc::vf2d	_pos = pos + vel * fElapsedTime + .5 * acc * abs(fElapsedTime) * fElapsedTime;

			vel = _vel;
			pos = _pos;
		}

		// Update just X of self
		void updateX(float fElapsedTime)
		{
			float	_vel = vel.x + acc.x * fElapsedTime;
			float	_pos = pos.x + vel.x * fElapsedTime + .5 * acc.x * abs(fElapsedTime) * fElapsedTime;

			vel.x = _vel;
			pos.x = _pos;
		}

		// Update just Y of self
		void updateY(float fElapsedTime)
		{
			float	_vel = vel.y + acc.y * fElapsedTime;
			float	_pos = pos.y + vel.y * fElapsedTime + .5 * acc.y * abs(fElapsedTime) * fElapsedTime;

			vel.y = _vel;
			pos.y = _pos;
		}

		// Return current values advanced by fTime
		pos2d advance(float fTime)
		{
			olc::vf2d	_acc = acc;
			olc::vf2d	_vel = vel + acc * fTime;
			olc::vf2d	_pos = pos + vel * fTime + .5 * acc * abs(fTime) * fTime;

			return pos2d(_pos, _vel, _acc);
		}

		// Return current values, with Just Y advanced by fTime
		pos2d advanceX(float fTime)
		{
			olc::vf2d	_acc = acc;
			olc::vf2d	_vel = { vel.x + acc.x * fTime, vel.y };
			olc::vf2d	_pos = { pos.x + vel.x * fTime + .5f * acc.x * abs(fTime) * fTime, pos.y };

			return pos2d(_pos, _vel, _acc);
		}

		// Return current values, with Just X advanced by fTime
		pos2d advanceY(float fTime)
		{
			olc::vf2d	_acc = acc;
			olc::vf2d	_vel = { vel.x, vel.y + acc.y * fTime };
			olc::vf2d	_pos = { pos.x, pos.y + vel.y * fTime + .5f * acc.y * abs(fTime) * fTime };

			return pos2d(_pos, _vel, _acc);
		}

		pos2d flip() { return pos2d(pos * FLIP, vel * FLIP, acc * FLIP); }

		pos2d offsetPos(olc::vf2d _offset) { return pos2d(pos - _offset, vel, acc); }

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
		bool		yLastColl;
		bool		xLastColl;

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
		virtual olc::vb2d isColliding(collider& _check, float fElapsedTIme, int debug = 0, std::string _name = "")
		{
			bool debug1 = debug == 1;
			bool debug2 = debug == 2;
			bool debug3 = debug == 3;

			olc::vf2d thisGeometricCenterPrevious = localityBuff.pos + size / 2.0f;
			olc::vf2d checkGeometricCenterPrevious = _check.localityBuff.pos + _check.size / 2.0f;

			olc::vf2d thisGeometricCenterCurrent = locality.pos + size / 2.0f;
			olc::vf2d checkGeometricCenterCurrent = _check.locality.pos + _check.size / 2.0f;

			olc::vf2d reach = _check.size / 2.0f + size / 2.0f;

			// Distance between the objecs with respect to their geometric centers
			olc::vf2d distanceFromCentersPrevious = checkGeometricCenterPrevious - thisGeometricCenterPrevious;
			olc::vf2d distanceFromCentersCurrent = checkGeometricCenterCurrent - thisGeometricCenterCurrent;

			// Checks on before/after X & Y if the objects have entered into each other's space
			olc::vb2d   collideTopRightPrevious = distanceFromCentersPrevious <= reach,
						collideBottomLeftPrevious = distanceFromCentersPrevious <= -reach;
			olc::vb2d   collideTopRightCurrent = distanceFromCentersCurrent <= reach,
						collideBottomLeftCurrent = distanceFromCentersCurrent <= -reach;
			
			olc::vb2d	collidePrevious = collideTopRightPrevious ^ collideBottomLeftPrevious;
			olc::vb2d	collideCurrent = collideTopRightCurrent ^ collideBottomLeftCurrent;

			// Spearation before collision, after collision is when they collide
			// Used for calculating when the collision occurs
			olc::vf2d separation = {
				-(distanceFromCentersPrevious.x + (distanceFromCentersPrevious.x > 0.0f ? -reach.x : +reach.x)),
				-(distanceFromCentersPrevious.y + (distanceFromCentersPrevious.y > 0.0f ? -reach.y : +reach.y)),
			};

			pos2d systemLocality = pos2d(separation, _check.localityBuff.vel + localityBuff.vel, _check.localityBuff.acc + localityBuff.acc);

			if (debug3)
			{
				/*

					olc::vf2d thisGeometricCenterPrevious = localityBuff.pos + size;
					olc::vf2d checkGeometricCenterPrevious = _check.localityBuff.pos + _check.size;

					olc::vf2d thisGeometricCenterCurrent = locality.pos + size;
					olc::vf2d checkGeometricCenterCurrent = _check.locality.pos + _check.size;

					olc::vf2d reach = _check.size / 2.0f + size / 2.0f;

					// Distance between the objecs with respect to their geometric centers
					olc::vf2d distanceFromCentersPrevious = checkGeometricCenterPrevious - thisGeometricCenterPrevious;
					olc::vf2d distanceFromCentersCurrent = checkGeometricCenterCurrent - thisGeometricCenterCurrent;

					// Checks on before/after X & Y if the objects have entered into each other's space
					olc::vb2d   collideTopRightPrevious = distanceFromCentersPrevious < reach,
								collideBottomLeftPrevious = distanceFromCentersPrevious < -reach;
					olc::vb2d   collideTopRightCurrent = distanceFromCentersCurrent < reach,
								collideBottomLeftCurrent = distanceFromCentersCurrent < -reach;
			
					olc::vb2d	collidePrevious = collideTopRightPrevious || collideBottomLeftPrevious;
					olc::vb2d	collideCurrent = collideTopRightCurrent || collideBottomLeftCurrent;

				*/

				std::cout << "thisGeoCenPrev:  "	<< thisGeometricCenterPrevious	<< " = " << localityBuff.pos		<< " + " << size		<< " / " << 2 << std::endl;
				std::cout << "checkGeoCenPrev: "	<< checkGeometricCenterPrevious << " = " << _check.localityBuff.pos << " + " << _check.size << " / " << 2 << std::endl;
				std::cout << "thisGeoCenCurr:  "	<< thisGeometricCenterCurrent	<< " = " << locality.pos			<< " + " << size		<< " / " << 2 << std::endl;
				std::cout << "checkGeoCenCurr: "	<< checkGeometricCenterCurrent	<< " = " << _check.locality.pos		<< " + " << _check.size << " / " << 2 << std::endl;

				std::cout << std::endl;

				std::cout << "reach: " << reach << " = " << _check.size << " / " << 2 << " + " << size << " / " << 2 << std::endl;

				std::cout << std::endl;

				std::cout << "distFromCenPrev: " << distanceFromCentersPrevious << " = " << checkGeometricCenterPrevious << " - " << thisGeometricCenterPrevious << std::endl;
				std::cout << "distFromCenCurr: " << distanceFromCentersCurrent  << " = " << checkGeometricCenterCurrent  << " - " << thisGeometricCenterCurrent  << std::endl;

				std::cout << std::endl;

				std::cout << "collTopRightPrev: "	<< collideTopRightPrevious		<< " = " << distanceFromCentersPrevious << " < " << reach  << std::endl;
				std::cout << "collBotLeftPrev:  "	<< collideBottomLeftPrevious	<< " = " << distanceFromCentersPrevious << " < " << -reach << std::endl;
				std::cout << "collTopRightCurr: "	<< collideTopRightCurrent		<< " = " << distanceFromCentersCurrent  << " < " << reach  << std::endl;
				std::cout << "collBotLeftCurr:  "	<< collideBottomLeftCurrent		<< " = " << distanceFromCentersCurrent  << " < " << -reach << std::endl;

				std::cout << std::endl;

				std::cout << "collPrevious: "	<<	collidePrevious << " = " <<	collideTopRightPrevious	<< " ^^ " << collideBottomLeftPrevious	<< std::endl;
				std::cout << "collCurrent:  "	<<  collideCurrent	<< " = " << collideTopRightCurrent	<< " ^^ " << collideBottomLeftCurrent	<< std::endl;
				//std::cout << "BcollPrevious: "  <<  (collideTopRightPrevious ^ collideBottomLeftPrevious) << " = " << collideTopRightPrevious << " ^^ " << collideBottomLeftPrevious	<< std::endl;
				//std::cout << "BcollCurrent:  "  <<  (collideTopRightCurrent ^ collideBottomLeftCurrent)  << " = " << collideTopRightCurrent  << " ^^ " << collideBottomLeftCurrent	<< std::endl;

				std::cout << std::endl;

				std::cout << "separation: " << separation << std::endl;
				std::cout << "systemLocality: \n" << systemLocality << "\n = \n" << _check.localityBuff << "\n + \n" << localityBuff << std::endl;

				std::cout << std::endl << std::endl << std::endl;

				//std::cout << ": " <<  << " = " <<  << " + " <<  << std::endl;
			}
			
			if(debug1)
				printf("Pos1: %s \t Pos2: %s \n Reach: %s \t Neg: %s \t Sep: %s \n Diff before: %s \t after: %s \n TR prev: %s \t TR curr: %s \n BL prev: %s \t BL curr: %s \n\n",
					localityBuff.pos.str().c_str(), _check.localityBuff.pos.str().c_str(),
					reach.str().c_str(), (-reach).str().c_str(), separation.str().c_str(),
					distanceFromCentersCurrent.str().c_str(), distanceFromCentersPrevious.str().c_str(),
					collideTopRightPrevious.str().c_str(), collideTopRightCurrent.str().c_str(),
					collideBottomLeftPrevious.str().c_str(), collideBottomLeftCurrent.str().c_str());

			if (debug1)
				std::cout << "AVG: \n" << systemLocality << std::endl;

			if (collideCurrent.x && collideCurrent.y)
			{
				if (!collidePrevious.x && collideCurrent.y && collidePrevious.y) // just collided
				{
					if (debug2)
						std::cout << "X COLLIDE" << std::endl;

					xLastColl = true;
					yLastColl = false;

					// Find the time of collison
					float timeX = timeOfIntercept (
						systemLocality.acc.x,
						systemLocality.vel.x,
						systemLocality.pos.x,
						true
					);

					locality.setX(localityBuff.advanceX(timeX));
					_check.locality.setX(_check.localityBuff.advanceX(timeX));

					// Bounce, temp
					locality.vel.x = -locality.vel.x / 2; // 
					_check.locality.vel.x = -_check.locality.vel.x / 2; // 

					if (timeX > 0.0f)
					{
						// Advance by remainder of time, unless touching the ground
						//locality.updateY(fElapsedTIme - timeY);
						//_check.locality.updateY(fElapsedTIme - timeY);
					}
				}
				else if (xLastColl && collidePrevious.x && collideCurrent.y && collidePrevious.y) // Sitting inside both previously and now (vertical sit)
				{
					if (debug2)
						std::cout << "X SITTING" << std::endl;

					float timeX = timeOfIntercept (
						systemLocality.acc.x,
						systemLocality.vel.x,
						systemLocality.pos.x,
						true
					);

					locality.setX(localityBuff.advanceX(timeX));
					_check.locality.setX(_check.localityBuff.advanceX(timeX));
				}
				else // Y is sitting, X is free (horizontal slide)
				{
					if (debug2)
						std::cout << "X FREE" << std::endl;

					locality.vel.x /= 1.05;
					_check.locality.vel.x /= 1.05;

					//locality.updateX(fElapsedTIme);
					//_check.locality.updateX(fElapsedTIme);
				}
			}

			if (collideCurrent.y && collideCurrent.x) // Currently intersecting the y-boundry
			{
				if (!collidePrevious.y && collideCurrent.x && collidePrevious.x) // just collided
				{
					if(debug2)
						std::cout << "Y COLLIDE" << std::endl;

					xLastColl = false;
					yLastColl = true;

					// Find the time of collison
					float timeY = timeOfIntercept (
						systemLocality.acc.y,
						systemLocality.vel.y,
						systemLocality.pos.y,
						true
					);

					locality.setY(localityBuff.advanceY(timeY));
					_check.locality.setY(_check.localityBuff.advanceY(timeY));

					// Bounce, temp
					locality.vel.y = -locality.vel.y / 2; // 
					_check.locality.vel.y = -_check.locality.vel.y / 2;
						
					if (timeY > 0.0f)
					{
						// Advance by remainder of time, unless touching the ground
						//locality.updateY(fElapsedTIme - timeY);
						//_check.locality.updateY(fElapsedTIme - timeY);
					}
				}
				else if (yLastColl && collidePrevious.y && collideCurrent.x && collidePrevious.x) // Sitting inside both previously and now (horizontal sit)
				{
					if (debug2)
						std::cout << "Y SITTING" << std::endl;

					float timeY = timeOfIntercept (
						systemLocality.acc.y,
						systemLocality.vel.y,
						systemLocality.pos.y,
						true
					);

					locality.setY(localityBuff.advanceY(timeY));
					_check.locality.setY(_check.localityBuff.advanceY(timeY));
				}
				else // X is sitting, Y is free (vertical slide)
				{
					if (debug2)
						std::cout << "Y FREE" << std::endl;

					locality.vel.y /= 1.05;
					_check.locality.vel.y /= 1.05;

					//locality.updateY(fElapsedTIme);
					//_check.locality.updateY(fElapsedTIme);
				}


				if (debug2)
					std::cout << std::endl;
			}
			
			return {false, false};
		}

		float timeOfIntercept(float acc, float vel, float pos, bool debug = false) // float acc, float vel, float pos
		{
			if(debug)
				std::cout << "Acc: " << acc << "   \tVel: " << vel << "\tPos: " << pos << std::endl;

			if (abs(acc) != 0.0f && abs(vel) != 0.0f) // If there is acceleration and velocity, use quadratic
			{
				// We only care about the positive solution
				olc::vf2d times = solveQuadratic(0.5f * acc, vel, pos);

				if(debug)
					std::cout << "  QUAD solution: " << (pos > 0 ? times.x : times.y) << " FULL: " << times << std::endl;
				
				return pos < 0 ? times.x : times.y;
			}
			else if (abs(acc) != 0.0f) // If just acceleration, use sqrt
			{
				olc::vf2d times =
				{
					 sqrt((abs(pos) * 2) / abs(acc)),
					-sqrt((abs(pos) * 2) / abs(acc))
				};

				if(debug)
					std::cout << "  SQRT solution: " << (pos > 0 ? times.x : times.y) << " FULL: " << times << std::endl;

				return pos < 0 ? times.x : times.y;
			}
			else if (abs(vel) != 0.0f) // else just divide
			{
				if(debug)
					std::cout << "  DIVD solution: " << pos / vel << std::endl;
				return pos / vel;
			}
			else
			{
				if(debug)
					std::cout << "  NONE solution: " << 0.0f << std::endl;
				return 0.0f;
			}
		}

		/*
		

			if (collYcurrent)
			{
				pos2d avg = _check.localityBuff.flip() - localityBuff.flip();

				if (collYcurrent && collXcurrent && collXbefore) //
				{
					// Check if it is sitting on the ground
					if (avg.pos.y <= reach.y + ACCURACY)
					{
						//if (!onGround)
							//std::cout << "On the ground!" << std::endl;
						//onGround = true;
					}
					else if (avg.pos.y > reach.y + ACCURACY)
					{
						//if (onGround)
							//std::cout << "In the air!" << std::endl;
						//onGround = false;
					}

					// Find the time of collison
					float timeY = timeOfIntercept(avg.acc.y, avg.vel.y, avg.pos.y - reach.y);

					//std::cout << "Y Pos: " << avg.pos.y << "\t  Y Reach: " << reach.y << "    Time: " << timeY << " Frame time: " << fElapsedTIme << std::endl;

					//if (!onGround)
					//{
						//std::cout << "Self localtiy: \n" << localityBuff << std::endl;
						//std::cout << "other locality:\n" << _check.localityBuff << std::endl;
						//std::cout << "diff locality: \n" << avg << std::endl;

						// Advance by timeY, pos will now be touching the other object

						std::cout << "BEF: \n" << locality << std::endl;
						locality.setY(localityBuff.advanceY(timeY));
						_check.locality.setY(_check.localityBuff.advanceY(timeY));
						std::cout << "AFT: \n" << locality << std::endl;

						// Bounce, temp
						locality.vel.y = abs(locality.vel.y / 2); //
						_check.locality.vel.y = abs(_check.locality.vel.y / 2); //

						if (timeY > 0.0f)
						{
							// Advance by remainder of time, unless touching the ground
							//locality.updateY(fElapsedTIme - timeY);
							//_check.locality.updateY(fElapsedTIme - timeY);
						}
					//}
					//else
					//{
					//	// Advance by timeY, counteract downword acceleration
					//	locality.setY(localityBuff.advanceY(timeY));
					//	_check.locality.setY(_check.localityBuff.advanceY(timeY));
					//}
				}
			}

		
		*/

		/*
		

		// Checks for rectangle collision by default
		virtual bool isColliding(object& _check, float fElapsedTIme)
		{
			olc::vf2d diffCurr = -locality.pos + _check.col.locality.pos;
			olc::vf2d reach = _check.col.size / 2 + size / 2;
			//olc::vf2d diffBuff = -localityBuff.pos + _check.col.localityBuff.pos;

			//bool collYbefore = abs(diffBuff.y) < reach.y, collYcurrent = abs(diffCurr.y) < reach.y;
			//bool collXbefore = abs(diffBuff.x) < reach.x, collXcurrent = abs(diffCurr.x) < reach.x;

			//std::cout << "Diff: \n" << diff << std::endl;

			if (abs(diffCurr.y) < reach.y)
			{
				pos2d avg = _check.col.localityBuff.flip() - localityBuff.flip();
				avg.pos.y -= reach.y;
				//std::cout << "Self localtiy: \n" << localityBuff << std::endl;
				//std::cout << "other locality:\n" << _check.col.localityBuff << std::endl;
				//std::cout << "diff locality: \n" << avg << std::endl;

				if (avg.pos.y == 0.0f) // reach.y Sitting on a surface
				{
					if(!onGround)
						std::cout << "On the ground!" << std::endl;
					onGround = true;
				}
				else
				{
					if (onGround)
						std::cout << "In the air!" << std::endl;
					onGround = false;
				}



				/*
				float timeY = 0.0f;
				if (avg.acc.y != 0.0f && avg.vel.y != 0.0f) // If there is acceleration and velocity, use quadratic
				{
					// We only care about the positive solution
					olc::vf2d timesY =  solveQuadratic(0.5f * avg.acc.y, avg.vel.y, avg.pos.y);

					std::cout << " Possible solutions: " << timesY << std::endl;
					timeY = std::max(timesY.x, timesY.y);
				}
				else if (avg.acc.y > 0.0f) // If just acceleration, use sqrt
					timeY = sqrt((avg.pos.y * 2) / avg.acc.y);
				else if(avg.vel.y > 0.0f) // else just divide
					timeY = avg.pos.y / avg.vel.y;
				
		float timeY = timeOfIntercept(avg.acc.y, avg.vel.y, avg.pos.y);

		// SOLVE: position they collided

		//std::cout << "Time of collision: quadratic: " << timeY << " liniar: " << abs(avg.pos.y / avg.vel.y) << " TOT: " << fElapsedTIme << std::endl;


		//if (onGround)
		//	locality = localityBuff;

		//if(!onGround)
		locality = localityBuff.advance(timeY);

		//std::cout << "New prev vel: \n" << locality << std::endl;

		if (!onGround)
			locality.vel.y = abs(locality.vel.y) / 2;

		//if(!onGround)
			//locality.update(fElapsedTIme - timeY);

		//std::cout << "New final vel: \n" << locality << std::endl << std::endl;

		return true;
			}


			return false;
		}

		
		*/

		/*
		

		// Checks for rectangle collision by default
		virtual bool isColliding(object& _check, float fElapsedTIme)
		{
			olc::vf2d reach = _check.col.size / 2 + size / 2;
			olc::vf2d diffCurr =-locality.pos + _check.col.locality.pos;
			olc::vf2d diffBuff = -localityBuff.pos + _check.col.localityBuff.pos;

			bool collYbefore = abs(diffBuff.y) < reach.y, collYcurrent = abs(diffCurr.y) < reach.y;
			bool collXbefore = abs(diffBuff.x) < reach.x, collXcurrent = abs(diffCurr.x) < reach.x;

			//std::cout << "Diff: \n" << diff << std::endl;

			if (collYcurrent || collXcurrent)
			{
				pos2d avg = _check.col.localityBuff.flip() - localityBuff.flip();


				if (collYcurrent && collXcurrent && collXbefore)
				{
					//std::cout << "Self localtiy: \n" << localityBuff << std::endl;
					//std::cout << "other locality:\n" << _check.col.localityBuff << std::endl;
					//std::cout << "diff locality: \n" << avg << std::endl;

					if (avg.pos.y == reach.y) // Sitting on a surface
					{
						//std::cout << "Im zero!" << std::endl;
						onGround = true;
						return true;
					}
					else
						onGround = false;

					/*
					float timeY = 0.0f;
					if (avg.acc.y != 0.0f && avg.vel.y != 0.0f) // If there is acceleration and velocity, use quadratic
					{
						// We only care about the positive solution
						olc::vf2d timesY =  solveQuadratic(0.5f * avg.acc.y, avg.vel.y, avg.pos.y);

						//std::cout << " Possible solutions: " << timesY << std::endl;
						timeY = std::max(timesY.x, timesY.y);
					}
					else if (avg.acc.y > 0.0f) // If just acceleration, use sqrt
						timeY = sqrt((avg.pos.y * 2) / avg.acc.y);
					else if(avg.vel.y > 0.0f) // else just divide
						timeY = avg.pos.y / avg.vel.y;
					
		float timeY = timeOfIntercept(avg.acc.y, avg.vel.y, avg.pos.y - reach.y);

		// SOLVE: position they collided

		//std::cout << "Time of collision: quadratic: " << timeY << " liniar: " << abs(avg.pos.y / avg.vel.y) << " TOT: " << fElapsedTIme << std::endl;

		locality = localityBuff.advance(timeY);

		//std::cout << "New prev vel: \n" << locality << std::endl;

		locality.vel.y = abs(locality.vel.y) / 2;

		locality.update(fElapsedTIme - timeY);

		//std::cout << "New final vel: \n" << locality << std::endl << std::endl;

		return true;
				}

			}


			return false;
		}

		float timeOfIntercept(float acc, float vel, float pos) // float acc, float vel, float pos
		{
			//float timeY = 0.0f;
			if (acc != 0.0f && vel != 0.0f) // If there is acceleration and velocity, use quadratic
			{
				// We only care about the positive solution
				olc::vf2d timesY = solveQuadratic(0.5f * acc, vel, pos);

				//std::cout << "  Possible solutions: " << timesY << std::endl;
				return std::max(timesY.x, timesY.y);
			}
			else if (acc > 0.0f) // If just acceleration, use sqrt
				return sqrt((pos * 2) / acc);
			else if (vel > 0.0f) // else just divide
				return pos / vel;
		}

		
		*/

		olc::vf2d solveQuadratic(float a, float b, float c)
		{
			if (a <= 0.000001f && a >= -0.000001f)
				return { 0,0 };

			olc::vf2d roots = solveSquareRoot((b * b) - 4 * a * c); // sqrt((b * b) - 4 * a * c);

			olc::vf2d out = { ((-b) + roots.x) / (2 * a), ((-b) + roots.y) / (2 * a) };

			if (isnan(out.x))
				out.x = 0.0f;

			if (isnan(out.y))
				out.y = 0.0f;

			return out;
		}

		olc::vf2d solveSquareRoot(float radicand)
		{
			if (abs(radicand) == 0.0f)
				return {0, 0};

			return {sqrt(radicand), -sqrt(radicand)};
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
			sim->FillRect (
				sim->worldToScreen(col.locality.pos + olc::vf2d{0.0f, col.size.y}),
				sim->worldToScreenScale(col.size)
			);
		}
	};
public:

	olc::vf2d screenToWorld(olc::vf2d _pos)
	{
		return (_pos - camPos - camOffset) * FLIP / zoom;
	}

	olc::vf2d worldToScreen(olc::vf2d _pos)
	{
		return _pos * FLIP * zoom + camPos + camOffset;
	}

	olc::vf2d screenToWorldScale(olc::vf2d _size)
	{
		return _size / zoom;
	}

	olc::vf2d worldToScreenScale(olc::vf2d _size)
	{
		return _size * zoom;
	}

	void drawGrid(float size)
	{
		for (int x = 0; x < round(SCREENWIDTH / zoom) + size; x += size)
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
		
		for (int y = 0; y < round(SCREENHEIGHT / zoom) + size; y += size)
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
	float zoom = DEFAULTZOOM;
	olc::vf2d camPos = { 0.0f, 0.0f };
	olc::vf2d camOffset = { SCREENWIDTH / 2, SCREENHEIGHT / 2 };

	bool pause = true;

	olc::vf2d dragInit = {0.0f, 0.0f};

	object block   =  object("block", olc::vf2d{ 0, 4.00f });
	object ground  = object("floor",  olc::vf2d{ -50.0f, -4 },    TYPES::RECT, olc::vf2d{ 100.0f, 1.0f });
	object ground2 = object("floor",  olc::vf2d{ -50.0f, 2 },     TYPES::RECT, olc::vf2d{ 100.0f, 2.0f });
	object wall    = object("wall",   olc::vf2d{ 10.0f, -50.0f }, TYPES::RECT, olc::vf2d{ 1.0f, 100.0f });

	bool OnUserCreate() override
	{
		block.col.locality.acc = {0.0f, -9.8f};
		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		std::cout << GetKey(olc::CAPS_LOCK).bHeld << std::endl;

		if (GetKey(olc::SPACE).bPressed)
			pause = !pause;

		if (GetMouseWheel())
		{
			zoom	+=	(GetMouseWheel() / 60.0f) / abs(DEFAULTZOOM / zoom);
			camPos	-=	(GetMouseWheel() / 60.0f) * ((olc::vf2d) GetMousePos() - (SCREENDIM / 2)) / 10.0f;
		}
		else if (GetMouse(0).bPressed) {
			dragInit = GetMousePos();
		}
		else if (GetMouse(0).bHeld)
		{
			camPos += GetMousePos() - dragInit;
			dragInit = GetMousePos();
		}

		if (GetKey(olc::R).bPressed)
		{
			camPos = { 0.0f, 0.0f };
			zoom = DEFAULTZOOM;
		}

		if (GetKey(olc::C).bPressed)
			camPos = block.col.locality.pos * zoom * olc::vf2d{-1, 1};

		if (pause)
		{
			Clear(olc::BLACK);

			drawGrid(1);
			block.drawSelf(this);
			ground.drawSelf(this);
			//ground2.drawSelf(this);
			wall.drawSelf(this);

			DrawString({ 0,0 }, screenToWorld(GetMousePos()).str());
		}

		if (pause && !GetKey(olc::D).bPressed && !GetKey(olc::A).bPressed) //  && !GetKey(olc::D).bHeld && !GetKey(olc::A).bHeld
			return true;

		if (GetKey(olc::A).bPressed) //  || GetKey(olc::A).bHeld
			fElapsedTime = -fElapsedTime;

		Clear(olc::BLACK);

		if (GetKey(olc::UP).bPressed)
			block.col.locality.vel.y = 5.0f;
		else if (GetKey(olc::DOWN).bPressed)
			block.col.locality.vel.y = -5.0f;
		
		if (GetKey(olc::RIGHT).bHeld)
			block.col.locality.vel.x += 50.0f * fElapsedTime;
		else if (GetKey(olc::LEFT).bHeld)
			block.col.locality.vel.x += -50.0f * fElapsedTime;

		block.update(fElapsedTime);
		ground.update(fElapsedTime);
		//ground2.update(fElapsedTime);
		wall.update(fElapsedTime);
		
		block.col.isColliding(ground.col, fElapsedTime, 0, "Ground 1");
		//block.col.isColliding(ground2.col, fElapsedTime, 3, "Ground 2");
		block.col.isColliding(wall.col, fElapsedTime, 3, "Floor");

		drawGrid(1);
		block.drawSelf(this);
		ground.drawSelf(this);
		//ground2.drawSelf(this);
		wall.drawSelf(this);

		DrawString({0,0}, screenToWorld(GetMousePos()).str());

		return true;
	}
};

int main()
{
	PhysicsSim sim;
	if (sim.Construct(SCREENWIDTH, SCREENHEIGHT, 2, 2, false, true))
		sim.Start();
	return 0;
}
