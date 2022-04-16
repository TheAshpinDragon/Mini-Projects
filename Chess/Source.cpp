#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"


#define WPWN new Piece(PAWN, WHITE, PieceLogic(PAWN))
#define WROK new Piece(ROOK, WHITE)
#define WKNT new Piece(KNIGHT, WHITE)
#define WBSP new Piece(BISHOP, WHITE)
#define WQUN new Piece(QUEEN, WHITE)
#define WKNG new Piece(KING, WHITE)

#define BPWN new Piece(PAWN, BLACK, PieceLogic(PAWN))
#define BROK new Piece(ROOK, BLACK)
#define BKNT new Piece(KNIGHT, BLACK)
#define BBSP new Piece(BISHOP, BLACK)
#define BQUN new Piece(QUEEN, BLACK)
#define BKNG new Piece(KING, BLACK)

#define NONE nullptr

#define SELECTED_COLOR olc::Pixel(0, 0, 255, 155)
#define MOVE_COLOR olc::Pixel(0, 255, 0, 155)
#define ATTACK_COLOR olc::Pixel(255, 0, 0, 155)

/*
CHESS PIECES SOURCE:
Date	13 January 2017
Source	https://commons.wikimedia.org/wiki/Category:PNG_chess_pieces/Standard_transparent
Author	Wikipedia user: Cburnett

CHESS BOARD SOURCE:
Date	1 October 2006
Source	https://commons.wikimedia.org/wiki/File:Chess_Board.svg
Author	Nevit Dilmen
*/

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

		asset(std::string _path, olc::vi2d _spriteSize) : path(_path), spriteSize(_spriteSize) {}

		void loadAsset(olc::PixelGameEngine* pge)
		{
			decal = new olc::Decal(new olc::Sprite(path));
		}
	};
}

int vtoi(olc::vi2d pos) { return pos.x + 8 * pos.y; }
struct vi2dPair {
	olc::vi2d a{}, b{};
};

class Chess : public olc::PixelGameEngine
{
public:
	Chess()
	{
		sAppName = "Lets Play Some Chess!";
	}

public:

	static void Log(std::string msg)
	{
		std::cout << msg << std::endl;
	}

	enum MoveType {
		FIXED,
		FIXED_AND_ATTACK,
		LINE,
		LINE_AND_ATTACK,
		// Pawn
		EN_PASSANT,
		DOUBLE_MOVE,
		RANK_UP,
		// King & rook
		CASTLE
	};

	static std::string moveToString(MoveType move)
	{
		switch (move)
		{
		case MoveType::FIXED:
			return "FIXED";
			break;
		case MoveType::FIXED_AND_ATTACK:
			return "FIXED_AND_ATTACK";
			break;
		case MoveType::LINE:
			return "LINE";
			break;
		case MoveType::LINE_AND_ATTACK:
			return "LINE_AND_ATTACK";
			break;
		case MoveType::EN_PASSANT:
			return "EN_PASSANT";
			break;
		case MoveType::DOUBLE_MOVE:
			return "DOUBLE_MOVE";
			break;
		case MoveType::RANK_UP:
			return "RANK_UP";
			break;
		case MoveType::CASTLE:
			return "CASTLE";
			break;
		}
	}

	struct Piece;
	struct GameBoard;
	struct PawnLogic;

	struct Move {
		olc::vi2d endPos;
		MoveType eType;
		Piece* secondaryPiece;

		Move(olc::vi2d endPosition, MoveType type) : endPos(endPosition), eType(type), secondaryPiece(nullptr) {}
		Move(olc::vi2d endPosition, MoveType type, Piece* other) : endPos(endPosition), eType(type), secondaryPiece(other) {}

		void drawSelf(Chess* pge) 
		{
			if (eType == MoveType::FIXED || eType == MoveType::DOUBLE_MOVE || eType == MoveType::EN_PASSANT)
			{
				pge->FillRectDecal(endPos * 64 + olc::vi2d(3, 3), { 64, 64 }, MOVE_COLOR);
			}

			if(eType == MoveType::FIXED_AND_ATTACK)
				pge->FillRectDecal(endPos * 64 + olc::vi2d(3, 3), { 64, 64 }, ATTACK_COLOR);
		}
	};

	enum PieceType {
		PAWN	= 5,
		BISHOP	= 4,
		KNIGHT	= 3,
		ROOK	= 2,
		KING	= 1,
		QUEEN	= 0
	};

	enum PieceColor {
		WHITE = 0,
		BLACK = 1
	};

	struct PieceLogic {
	public:
		bool firstMove;
		PieceType eType;

	private:

		std::vector<Move> pawnLogic(Piece* piece, GameBoard& board) 
		{
			std::vector<Move> moves;
			olc::vi2d pos = piece->pos;
			int forword = piece->eColor == PieceColor::WHITE ? -1 : +1;
			PieceColor enemy = piece->eColor == PieceColor::WHITE ? PieceColor::BLACK : PieceColor::WHITE;

			// Single move forword - Must have one empty space
			if (board.bounded({ pos.x, pos.y + forword }) && !board.isPieceAt({ pos.x, pos.y + forword }))
			{
				moves.push_back(Move({ pos.x, pos.y + forword }, MoveType::FIXED));

				// Double move forword - Must have one additional empty space
				if (firstMove && !board.isPieceAtBounded({ pos.x, pos.y + 2 }))
					moves.push_back(Move({ pos.x, pos.y + 2 * forword }, MoveType::FIXED));
			}

			// Attack left
			if (board.isPieceAtBounded({ pos.x - 1, pos.y + forword }, enemy))
				moves.push_back(Move({ pos.x - 1, pos.y + forword }, MoveType::FIXED_AND_ATTACK, board.getPieceAt({ pos.x - 1, pos.y + forword })));
			// Attack right
			if (board.isPieceAtBounded({ pos.x + 1, pos.y + forword }, enemy))
				moves.push_back(Move({ pos.x + 1, pos.y + forword }, MoveType::FIXED_AND_ATTACK, board.getPieceAt({ pos.x + 1, pos.y + forword })));

			// En Passant left
			/*if (board.bounded  ({ pos.x - 1, pos.y + forword }) &&
				board.isPieceAtBounded({ pos.x - 1, pos.y }, enemy) &&
				board.getPieceAt({ pos.x - 1, pos.y })->eType == PieceType::PAWN &&
				board.getPieceAt({ pos.x - 1, pos.y })->logic.doubleMoved)
					moves.push_back(Move({ pos.x - 1, pos.y + forword }, MoveType::EN_PASSANT, board.getPieceAt({ pos.x - 1, pos.y })));
			// En Passant right
			if (board.bounded  ({ pos.x + 1, pos.y + forword }) &&
				board.isPieceAtBounded({ pos.x + 1, pos.y }, enemy) &&
				board.getPieceAt({ pos.x - 1, pos.y })->eType == PieceType::PAWN &&
				board.getPieceAt({ pos.x - 1, pos.y })->logic.doubleMoved)
					moves.push_back(Move({ pos.x + 1, pos.y + forword }, MoveType::EN_PASSANT, board.getPieceAt({ pos.x + 1, pos.y })));*/

			// Rank up if desired
			if (!board.bounded({ pos.x, pos.y + forword }))
				moves.push_back(Move({ pos.x, pos.y }, MoveType::RANK_UP));

			Log("Moves: " + std::to_string(moves.size()));

			for (Move m : moves)
				Log("  " + moveToString(m.eType));

			return moves;
		}

		std::vector<Move> bishopLogic(Piece* piece, GameBoard& board)
		{
			std::vector<Move> moves;
			olc::vi2d pos = piece->pos;
			int forword = piece->eColor == PieceColor::WHITE ? -1 : +1;
			PieceColor enemy = piece->eColor == PieceColor::WHITE ? PieceColor::BLACK : PieceColor::WHITE;

			return moves;
		}

		std::vector<Move> knightLogic(Piece* piece, GameBoard& board)
		{
			std::vector<Move> moves;
			olc::vi2d pos = piece->pos;
			int forword = piece->eColor == PieceColor::WHITE ? -1 : +1;
			PieceColor enemy = piece->eColor == PieceColor::WHITE ? PieceColor::BLACK : PieceColor::WHITE;

			return moves;
		}

		std::vector<Move> rookLogic(Piece* piece, GameBoard& board)
		{
			std::vector<Move> moves;
			olc::vi2d pos = piece->pos;
			int forword = piece->eColor == PieceColor::WHITE ? -1 : +1;
			PieceColor enemy = piece->eColor == PieceColor::WHITE ? PieceColor::BLACK : PieceColor::WHITE;

			return moves;
		}

		std::vector<Move> kingLogic(Piece* piece, GameBoard& board)
		{
			std::vector<Move> moves;
			olc::vi2d pos = piece->pos;
			int forword = piece->eColor == PieceColor::WHITE ? -1 : +1;
			PieceColor enemy = piece->eColor == PieceColor::WHITE ? PieceColor::BLACK : PieceColor::WHITE;

			return moves;
		}

		std::vector<Move> queenLogic(Piece* piece, GameBoard& board)
		{
			std::vector<Move> moves;
			olc::vi2d pos = piece->pos;
			int forword = piece->eColor == PieceColor::WHITE ? -1 : +1;
			PieceColor enemy = piece->eColor == PieceColor::WHITE ? PieceColor::BLACK : PieceColor::WHITE;

			return moves;
		}

	public:
		PieceLogic(PieceType type) : firstMove( true ), eType( type ) { }

		std::vector<Move> runLogic(Piece* piece, GameBoard& board)
		{
			switch (eType)
			{
			case PAWN:
				return pawnLogic(piece, board);
				break;
			case BISHOP:
				//return pawnLogic(piece, board);
				return std::vector<Move>();
				break;
			case KNIGHT:
				//return pawnLogic(piece, board);
				return std::vector<Move>();
				break;
			case ROOK:
				//return pawnLogic(piece, board);
				return std::vector<Move>();
				break;
			case KING:
				//return pawnLogic(piece, board);
				return std::vector<Move>();
				break;
			case QUEEN:
				//return pawnLogic(piece, board);
				return std::vector<Move>();
				break;
			default:
				return std::vector<Move>();
			}
		}
	};

	struct Piece {
		olc::vi2d pos;
		std::vector<Move> moves;
		PieceType eType;
		PieceColor eColor;
		PieceLogic logic;
		bool displayMoves;

		Piece(PieceType type, PieceColor color)
			: eType{ type }, eColor{ color }, logic( PieceLogic( type ) ), pos{ 0,0 }, displayMoves{ false } {}

		Piece(PieceType type, PieceColor color, PieceLogic piecelogic)
			: eType{ type }, eColor{ color }, logic{ piecelogic }, pos{ 0,0 }, displayMoves{ false } {}

		Piece* operator= (Piece* rhs)
		{
			eType = rhs->eType;
			eColor = rhs->eColor;
			pos = {0,0};
			//moves = rhs->moves;
		}

		void drawSelf(Chess* pge) {
			pge->DrawDecal(pos * 64, pge->chessPieceSheet.decals[eType][eColor]);

			if (displayMoves)
			{
				for (Move m : moves)
					m.drawSelf(pge);
			}
		}

		bool tryMoveTo(GameBoard* board, olc::vi2d tryPos)
		{
			for(Move m : moves)
				if (m.endPos == tryPos)
				{
					// No longer first move
					logic.firstMove = false;

					// If there is a second piece effected
					if (m.secondaryPiece)
					{
						// If the move is an attack move, delete the piece in question
						if (m.eType != MoveType::CASTLE)
							board->deletePieceAt(m.secondaryPiece->pos);
					}

					// Move to new location on the board and internally
					board->movePiece(pos, tryPos);
					pos = tryPos;

					// Update logic based on new position
					board->updateAllLogic();

					return true;
				}

			return false;
		}

		void updLogic(GameBoard* board)
		{
			moves = logic.runLogic(this, *board);
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
		olc::vi2d selectedPiece;
		bool isPieceSelected;
		PieceColor eColor;
		int turn;

		GameBoard()
		{
			board = defaultBoard;
			isPieceSelected = false;
			eColor = PieceColor::WHITE;

			for(int i = 0; i < 8; i++)
				for (int k = 0; k < 8; k++)
					if(board[vtoi({ k, i })])
						board[vtoi({ k, i })]->pos = { k, i };


			for (int i = 0; i < 8; i++)
				for (int k = 0; k < 8; k++)
					if (board[vtoi({ k, i })] && board[vtoi({ k, i })]->eType == PieceType::PAWN)
						board[vtoi({ k, i })]->updLogic(this);
		}

		bool bounded(olc::vi2d pos) { return pos.x >= 0 && pos.y >= 0 && pos.x < 8 && pos.y < 8; }
		Piece* getPieceAt(olc::vi2d pos) { return bounded(pos) ? board[vtoi(pos)] : nullptr; }

		bool isPieceAtBounded(olc::vi2d pos) { return bounded(pos) && board[vtoi(pos)]; }
		bool isPieceAtBounded(olc::vi2d pos, PieceColor color) { Piece* p = getPieceAt(pos); return p && p->eColor == color; }
		bool isPieceAt(olc::vi2d pos) { return board[vtoi(pos)]; }
		bool isPieceAt(olc::vi2d pos, PieceColor color) { return board[vtoi(pos)]->eColor == color; }

		void deletePieceAt(olc::vi2d pos) { if(bounded(pos)) board[vtoi(pos)] = nullptr; }
		void movePiece(olc::vi2d from, olc::vi2d to) { if (getPieceAt(from) && !getPieceAt(to)) { board[vtoi(to)] = getPieceAt(from); deletePieceAt(from); } }
		void updateAllLogic() { for (Piece* p : board) if(p) p->updLogic(this);  }

		vi2dPair findOnHorizontal(olc::vi2d start)
		{
			bool endR = false, endL = false;
			vi2dPair out;

			for (int i = 1; !endR || !endL; i++)
			{
				// Never runs into a piece, vector stays as -1, -1
				if (!endR && !bounded({ start.x + i, start.y }))
					endR = true;
				// Runs into a piece, vector becomes that position
				else if (!endR && isPieceAt({ start.x + i, start.y }))
				{
					out.a = { start.x + i, start.y };
					endR = true;
				}

				// Never runs into a piece, vector stays as -1, -1
				if (!endL && !bounded({ start.x - i, start.y }))
					endL = true;
				// Runs into a piece, vector becomes that position
				else if (!endL && isPieceAt({ start.x - i, start.y }))
				{
					out.b = { start.x - i, start.y };
					endL = true;
				}
			}

			return out;
		}

		vi2dPair findOnVertical(olc::vi2d start)
		{

			bool endD = false, endU = false;
			vi2dPair out;

			for (int i = 1; !endD || !endU; i++)
			{
				// Never runs into a piece, vector stays as -1, -1
				if (!endD && !bounded({ start.x, start.y + i }))
					endD = true;
				// Runs into a piece, vector becomes that position
				else if (!endD && isPieceAt({ start.x, start.y + i }))
				{
					out.a = { start.x, start.y + i };
					endD = true;
				}

				// Never runs into a piece, vector stays as -1, -1
				if (!endU && !bounded({ start.x, start.y - i }))
					endU = true;
				// Runs into a piece, vector becomes that position
				else if (!endU && isPieceAt({ start.x, start.y - i }))
				{
					out.b = { start.x, start.y - i };
					endU = true;
				}
			}

			return out;
		}

		olc::vi2d screenToBoard(olc::vi2d pos) { return { pos.x / 64, pos.y / 64 }; }

		void checkInput(Chess* pge)
		{
			if (pge->GetMouse(0).bPressed)
			{
				olc::vi2d pos = screenToBoard(pge->GetMousePos());

				// If the selection is out of range, return
				if (!bounded(pos))
					return;

				// Get the new selection
				Piece* selected = getPieceAt(pos);

				if (!isPieceSelected)
				{
					// If there is no piece or if the piece color is not the current player's color, return
					if (!selected || selected->eColor != eColor)
						return;

					// Else, set the position and that a piece is selected
					selectedPiece = pos;
					isPieceSelected = true;
					selected->displayMoves = true;
				}
				else if (isPieceSelected)
				{
					// Reset selected
					isPieceSelected = false;
					getPieceAt(selectedPiece)->displayMoves = false;

					// If you just select the same square twice
					if (pos == selectedPiece)
						return;

					// If it is a valid move
					if (getPieceAt(selectedPiece)->tryMoveTo(this, pos))
					{
						turn++;
						eColor = eColor == WHITE ? BLACK : WHITE;
						return;
					}

					// If it is an invalid move that is not the selected square, try to select the new square
					checkInput(pge);
				}
			}
		}

		void drawBoard(Chess* pge)
		{
			checkInput(pge);

			for (Piece* p : board)
				if(p)
					p->drawSelf(pge);

			if (isPieceSelected)
			{
				pge->FillRectDecal(selectedPiece * 64 + olc::vi2d(3, 3), { 64, 64 }, olc::Pixel(0, 0, 255, 155));
			}
		}
	};

public:
	ASSETS::sheet chessPieceSheet = ASSETS::sheet("ChessPiecesArray.png", { 64, 64 }, { 2, 6 }, true);
	ASSETS::asset chessBoardPNG = ASSETS::asset("ChessBoard.png", { 640, 640 });

	GameBoard board;

	bool OnUserCreate() override
	{
		chessPieceSheet.loadAsset(this);
		chessBoardPNG.loadAsset(this);

		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		Clear(olc::BACK);
		DrawDecal({ 0,0 }, chessBoardPNG.decal, {0.81f, 0.81f}, olc::Pixel(220, 220, 220));
		board.drawBoard(this);

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