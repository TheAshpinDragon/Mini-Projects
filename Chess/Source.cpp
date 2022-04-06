#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"


#define WPWN new Piece(PAWN, WHITE)
#define WROK new Piece(ROOK, WHITE)
#define WKNT new Piece(KNIGHT, WHITE)
#define WBSP new Piece(BISHOP, WHITE)
#define WQUN new Piece(QUEEN, WHITE)
#define WKNG new Piece(KING, WHITE)

#define BPWN new Piece(PAWN, BLACK)
#define BROK new Piece(ROOK, BLACK)
#define BKNT new Piece(KNIGHT, BLACK)
#define BBSP new Piece(BISHOP, BLACK)
#define BQUN new Piece(QUEEN, BLACK)
#define BKNG new Piece(KING, BLACK)

#define NONE nullptr

/*
CHESS PIECES SOURCE:
Date	13 January 2017
Source	https://commons.wikimedia.org/wiki/Category:PNG_chess_pieces/Standard_transparent
Author	Wikipedia user: Cburnett
*/

class Chess : public olc::PixelGameEngine
{
public:
	Chess()
	{
		// Name your application
		sAppName = "Lets Play Some Chess!";
	}

public:

	enum MoveType {
		FIXED_AND_ATTACK,
		FIXED,
		LINE,
		// Pawn
		EN_PASSANT,
		DOUBLE_MOVE,
		RANK_UP,
		// King & rook
		CASTLE
	};

	struct Move {
		olc::vi2d newPos;
		MoveType eType;
	};

	enum PieceType {
		PAWN,
		ROOK,
		KNIGHT,
		BISHOP,
		QUEEN,
		KING
	};

	enum PieceColor {
		WHITE,
		BLACK
	};

	struct PieceLogic {
		
	};

	struct Piece {
		PieceType eType;
		PieceColor eColor;
		std::vector<Move> moves;

		Piece(PieceType type, PieceColor color)
			: eType{ type }, eColor{ color } {}

		Piece* operator= (Piece* rhs)
		{
			eType = rhs->eType;
			eColor = rhs->eColor;
			//moves = rhs->moves;
		}
	};

	struct GameBoard {
		const std::vector<Piece*> defaultBoard {
		//    A     B     C     D     E     F     G     H
			BROK, BKNT, BBSP, BQUN, BKNG, BBSP, BKNT, BROK, // 8
			BPWN, BPWN, BPWN, BPWN, BPWN, BPWN, BPWN, BPWN, // 7
			NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, // 6
			NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, // 5
			NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, // 4
			NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, // 3
			WPWN, WPWN, WPWN, WPWN, WPWN, WPWN, WPWN, WPWN, // 2
			WROK, WKNT, WBSP, WQUN, WKNG, WBSP, WKNT, WROK, // 1
		};

		std::vector<Piece*> board;
		int turn;

		GameBoard() : board(defaultBoard)
		{}
	};

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
	Chess chessGame;
	if (chessGame.Construct(640, 640, 2, 2))
		chessGame.Start();
	return 0;
}