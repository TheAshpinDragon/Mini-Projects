#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

#define FLIP_PERSPECTIVE	(olc::vf2d{-1,-1})
#define FLIP_X(p)			((p) * olc::vf2d{-1,1})
#define FLIP_Y(p)			((p) * olc::vf2d{1,-1})
#define SCREEN_SIZE			(olc::vi2d{256, 240} * 2)
#define SCREEN_OFFSET		(olc::vi2d{256, 240})

class WorldAndCamera : public olc::PixelGameEngine
{
public:
	WorldAndCamera()
	{
		sAppName = "World And Camera Demo";
	}

public:

	// A generic object with a position and shape
	class obj {
	public:
		olc::vf2d pos;
		int32_t radius;

		obj(olc::vf2d objPos, int32_t objRadius) : pos{ objPos }, radius{ objRadius } {}

		void draw(WorldAndCamera* pge, olc::vf2d camPos, float zoom)
		{
			// Shape
			pge->FillCircle(pge->getWorldToScreen(pos), radius * zoom);

			// Position decal (text relitive to position)
			pge->DrawStringDecal(
				pge->getWorldToScreen(pos) + olc::vf2d{ radius * zoom + 2, -7 },
				pos.str(),
				olc::WHITE, olc::vf2d{ 0.5f, 2.0f });
		}
	};

	// Varaibles
	std::vector<obj> objs;
	olc::vf2d camPos = { 0,0 }; // The offset of the manipulatable camera or scene
	float zoom = 1.0f; // The vector scalar
	bool perspective = true; // |{ false = move the camera (bird's eye view) }|{ true = move the scene (like draging an image) }|

	// Convert world coordinates (centered, +up, -down, unscaled) to screen coordinates (top left, -up, +down, scaled)
	olc::vf2d getWorldToScreen(olc::vf2d pos = { 0,0 })
	{
		// Flip the y of the world space to match the axis of the screen space
		olc::vf2d objectInScreenSpace = FLIP_Y(pos);

		// Flip the offset's x to pan with camera, flip the offset's y to pan the scene
		olc::vf2d offsetInScreenSpace = perspective ? FLIP_Y(camPos) : FLIP_X(camPos);

		// Add the offset to the object and scale it (the offset is the focus (0,0), the further you get from it the more you are displaced)
		olc::vf2d zoomedScreenSpace = (objectInScreenSpace + offsetInScreenSpace) * zoom;

		// Finally add the SCREEN_OFFSET to match the position of (0,0) in screen space (top left)
		olc::vf2d uncenteredScreenSpace = zoomedScreenSpace + SCREEN_OFFSET;

		// Consolidated:
		return ((FLIP_Y(pos) + (perspective ? FLIP_Y(camPos) : FLIP_X(camPos))) * zoom) + SCREEN_OFFSET;
	}

	// Convert screen coordinates (top left, -up, +down, scaled) to world coordinates (centered, +up, -down, unscaled)
	olc::vf2d getScreenToWorld(olc::vf2d pos = { 0,0 })
	{
		// First subtract the SCREEN_OFFSET to match the position of (0,0) in world space (center)
		olc::vf2d centeredScreenSpace = pos - SCREEN_OFFSET;

		// Unscale it by deviding by the zoom
		olc::vf2d unzoomedScreenSpace = centeredScreenSpace / zoom;

		// Flip the offset's x to pan with camera, flip the offset's y to pan the scene
		olc::vf2d offsetInScreenSpace = perspective ? FLIP_Y(camPos) : FLIP_X(camPos);

		// Remove the offset and flip the y of the screen space to match the axis of the world space
		olc::vf2d objectInWorldSpace = FLIP_Y(unzoomedScreenSpace - offsetInScreenSpace);

		// Consolidated:
		return FLIP_Y(((pos - SCREEN_OFFSET) / zoom) - (perspective ? FLIP_Y(camPos) : FLIP_X(camPos)));
	}

	bool OnUserCreate() override
	{
		// Initialize objects
		objs.push_back(obj({ 43, 22 }, 10));
		objs.push_back(obj({ -62, 36 }, 6));
		objs.push_back(obj({ 21, -26 }, 12));
		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		Clear(olc::BLACK);

		// User input
		{
			// Move camera
			if (GetKey(olc::W).bHeld)
				camPos.y += 1.0f / zoom;
			else if (GetKey(olc::S).bHeld)
				camPos.y -= 1.0f / zoom;

			if (GetKey(olc::D).bHeld)
				camPos.x += 1.0f / zoom;
			else if (GetKey(olc::A).bHeld)
				camPos.x -= 1.0f / zoom;

			// Zoom
			if (GetMouseWheel() && 1.0f)
				zoom *= GetMouseWheel() > 0 ? 1.1f : 0.9f;

			// Display info
			DrawStringDecal({ 0,(float)SCREEN_SIZE.y - 16 },
				"ZOOM: " + std::to_string(zoom) + "\t"
				"CAM-POS: " + camPos.str(),
				olc::WHITE, olc::vf2d{ 0.25f, 1 } *2);
		}

		// Draw to the screen
		{
			// Draw objects
			for (auto o : objs) o.draw(this, camPos, zoom);

			// World center
			FillCircle(getWorldToScreen(), 1);
			DrawStringDecal(getWorldToScreen() + olc::vi2d{ 8,-7 }, "(0,0)", olc::CYAN, olc::vf2d{ 0.5f, 2 });

			// Screen center
			FillCircle(SCREEN_OFFSET, 1, olc::DARK_GREY);
			FillRectDecal(SCREEN_OFFSET + olc::vi2d{ 8,-8 }, olc::vf2d{ (float)getScreenToWorld(SCREEN_OFFSET).str().length() * 4, 16 }, olc::GREY);
			DrawStringDecal(SCREEN_OFFSET + olc::vi2d{ 8,-7 }, getScreenToWorld(SCREEN_OFFSET).str(), olc::CYAN, olc::vf2d{ 0.5f, 2 });

			// Screen lower right
			FillCircle(SCREEN_OFFSET + SCREEN_OFFSET / 2, 1, olc::DARK_GREY);
			FillRectDecal(SCREEN_OFFSET + SCREEN_OFFSET / 2 + olc::vi2d{ 8,-8 }, olc::vf2d{ (float)getScreenToWorld(SCREEN_OFFSET + SCREEN_OFFSET / 2).str().length() * 4, 16 }, olc::GREY);
			DrawStringDecal(SCREEN_OFFSET + SCREEN_OFFSET / 2 + olc::vi2d{ 8,-7 }, getScreenToWorld(SCREEN_OFFSET + SCREEN_OFFSET / 2).str(), olc::CYAN, olc::vf2d{ 0.5f, 2 });
		}

		return true;
	}
};

int main()
{
	WorldAndCamera demo;
	if (demo.Construct(SCREEN_SIZE.x, SCREEN_SIZE.y, 2, 2, false, true))
		demo.Start();
	return 0;
}