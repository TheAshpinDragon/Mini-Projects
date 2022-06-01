#include "olcPixelGameEngine.h"

namespace ASSETS {
	struct sheet {
		std::string path;
		olc::vi2d rowColumn, spriteSize;
		std::vector<std::vector<olc::Decal*>> decals;

		sheet(std::string _path, olc::vi2d _spriteSize, olc::vi2d _rowColumn, bool _flipRowColumn = false) {
			path = _path;
			spriteSize = _spriteSize;

			if (_flipRowColumn) {
				rowColumn.x = _rowColumn.y;
				rowColumn.y = _rowColumn.x;
			}
			else
				rowColumn = _rowColumn;
		}

		void loadAsset(olc::PixelGameEngine* pge) {
			// Load and process sheet
			olc::Sprite* sheet = new olc::Sprite(path);
			olc::Sprite* screen = pge->GetDrawTarget();

			for (int i = 0; i < rowColumn.x; i++)
			{
				decals.push_back(std::vector<olc::Decal*>());
				for (int k = 0; k < rowColumn.y; k++)
				{
					olc::Sprite* buffer = new olc::Sprite(spriteSize.x, spriteSize.y);
					pge->SetDrawTarget(buffer);

					pge->DrawPartialSprite({ 0,0 }, sheet, olc::vi2d{ spriteSize.x * i, spriteSize.y * k }, spriteSize);
					decals[i].push_back(new olc::Decal(buffer));
				}
			}

			pge->SetDrawTarget(screen);
		}
	};

	struct asset {
		std::string path;
		olc::vi2d spriteSize;
		olc::Decal* decal;

		asset(std::string _path, olc::vi2d _spriteSize) : path(_path), spriteSize(_spriteSize), decal(nullptr) {}

		void loadAsset(olc::PixelGameEngine* pge)
		{
			decal = new olc::Decal(new olc::Sprite(path));
		}
	};
}

// Override base class with your custom functionality
class TicTacToe : public olc::PixelGameEngine
{
public:
	TicTacToe()
	{
		sAppName = "Lets play Tic-Tac-Toe!";
	}

public:

	enum class piece : char {
		X = 'X',
		O = 'O',
		N = ' '
	};

	ASSETS::sheet assetSheet = ASSETS::sheet("Tic-Tac-Toe_Sheet.png", { 16, 16 }, { 3, 1 });

	int xScore = 0, oScore = 0;
	piece currentPlayer = piece::X;
	std::vector<std::vector<piece>> board = {
		{ piece::N, piece::N, piece::N },
		{ piece::N, piece::N, piece::N },
		{ piece::N, piece::N, piece::N },
	};

	// Draw the screen
	void drawBoard()
	{
		Clear(olc::WHITE);

		// Score
		DrawStringDecal(olc::vi2d{ 1,0 }, "X:" + std::to_string(xScore) + "  O:" + std::to_string(oScore), olc::BLACK, olc::vf2d{ 0.7, 1 });

		// Current player
		FillRectDecal(olc::vf2d{ 19,-0.5 }, { 8.0f, 8.0f }, olc::RED);
		DrawStringDecal(olc::vf2d{ 20.5,0.5f }, std::string(1, (char)currentPlayer), olc::WHITE, olc::vf2d{ 0.7, 0.95 });

		// Cells
		for (int i = 0; i < 3; i++)
			for (int k = 0; k < 3; k++)
			{
				DrawDecal(olc::vi2d{ 15 * i, 15 * k + 8 }, assetSheet.decals[2][0]);

				piece pieceOnCell = board[k][i];

				if (pieceOnCell != piece::N)
					DrawDecal(olc::vf2d{ 15 * i + 0.5f, 15 * k + 8 + 0.5f }, assetSheet.decals[pieceOnCell == piece::X ? 0 : 1][0], olc::vf2d{ 0.9375, 0.9375 });
			}
	}

	// Check for a winner
	piece checkBoard()
	{
		// Horizontal
		for (int i = 0; i < 3; i++)
		{
			int avg = ((int)board[i][0] + (int)board[i][1] + (int)board[i][2]) / 3;
			
			if (avg == (int)'X') return piece::X;
			if (avg == (int)'O') return piece::O;
		}

		// Vertical
		for (int i = 0; i < 3; i++)
		{
			int avg = ((int)board[0][i] + (int)board[1][i] + (int)board[2][i]) / 3;

			if (avg == (int)'X') return piece::X;
			if (avg == (int)'O') return piece::O;
		}

		// Diagonal
		{
			int avgA = ((int)board[0][0] + (int)board[1][1] + (int)board[2][2]) / 3;
			int avgB = ((int)board[2][0] + (int)board[1][1] + (int)board[0][2]) / 3;

			if (avgA == (int)'X' || avgB == (int)'X') return piece::X;
			if (avgA == (int)'O' || avgB == (int)'O') return piece::O;
		}

		return piece::N;
	}

	// Set all cells to none
	void clearBoard() 
	{
		for (int i = 0; i < 3; i++)
			for (int k = 0; k < 3; k++)
				board[i][k] = piece::N;
	}

	bool OnUserCreate() override
	{
		assetSheet.loadAsset(this);
		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		if (GetKey(olc::R).bPressed)
		{
			clearBoard();
			xScore = 0;
			oScore = 0;
			currentPlayer = piece::X;
		}

		if(GetMouse(0).bPressed)
		{
			// Get the cell the mouse is over
			olc::vi2d cellClicked = (GetMousePos() - olc::vi2d{ 0,8 }) / olc::vi2d{15, 15};
			
			if (board[cellClicked.y][cellClicked.x] == piece::N)
			{
				// Set selected cell
				board[cellClicked.y][cellClicked.x] = currentPlayer;
				
				// Switch player
				currentPlayer = currentPlayer == piece::X ? piece::O : piece::X;
				
				// Check for a winner
				piece winner = checkBoard();
				if (winner != piece::N)
				{
					clearBoard();
					xScore += winner == piece::X;
					oScore += winner == piece::O;
				}
			}
		}

		drawBoard();

		return true;
	}
};

int main()
{
	TicTacToe demo;
	if (demo.Construct(46, 54, 8, 8))
		demo.Start();
	return 0;
}