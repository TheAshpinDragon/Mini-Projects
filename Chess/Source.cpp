#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"


#define WPWN new Piece(PieceType::PAWN, PieceColor::WHITE, PieceLogic(PieceType::PAWN))
#define WROK new Piece(PieceType::ROOK, PieceColor::WHITE, PieceLogic(PieceType::ROOK))
#define WKNT new Piece(PieceType::KNIGHT, PieceColor::WHITE)
#define WBSP new Piece(PieceType::BISHOP, PieceColor::WHITE, PieceLogic(PieceType::BISHOP))
#define WQUN new Piece(PieceType::QUEEN, PieceColor::WHITE)
#define WKNG new Piece(PieceType::KING, PieceColor::WHITE)

#define BPWN new Piece(PieceType::PAWN, PieceColor::BLACK, PieceLogic(PieceType::PAWN))
#define BROK new Piece(PieceType::ROOK, PieceColor::BLACK, PieceLogic(PieceType::ROOK))
#define BKNT new Piece(PieceType::KNIGHT, PieceColor::BLACK)
#define BBSP new Piece(PieceType::BISHOP, PieceColor::BLACK, PieceLogic(PieceType::BISHOP))
#define BQUN new Piece(PieceType::QUEEN, PieceColor::BLACK)
#define BKNG new Piece(PieceType::KING, PieceColor::BLACK)

#define BLNK nullptr

#define SELECTED_COLOR olc::Pixel(0, 0, 255, 155)
#define MOVE_COLOR olc::Pixel(0, 255, 0, 155)
#define ATTACK_COLOR olc::Pixel(255, 0, 0, 155)

#define min(a, b) ((a < b) ? a : b)
#define max(a, b) ((a > b) ? a : b)

namespace Debug {
	static char DebugPieceLogic = 0;

	char Pawn	= 0b00000001;
	char Bishop = 0b00000010;
	char Knight = 0b00000100;
	char Rook	= 0b00001000;
	char King	= 0b00010000;
	char Queen	= 0b00100000;
};

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

		asset(std::string _path, olc::vi2d _spriteSize) : path(_path), spriteSize(_spriteSize), decal( nullptr ){}

		void loadAsset(olc::PixelGameEngine* pge)
		{
			decal = new olc::Decal(new olc::Sprite(path));
		}
	};
}

namespace Game {
	
	template <class T>
	struct v2dPair {
		olc::v2d_generic<T> a{}, b{};
		
	};

	typedef v2dPair<int32_t> vi2dPair;
	typedef v2dPair<bool> vb2dPair;

	template <class T, class U>
	struct pair {
		T a;
		U b;

		pair(T first, U seccond) : a(first), b(seccond) {}
	};
}

class Chess : public olc::PixelGameEngine
{
public:
	Chess()
	{
		sAppName = "Lets Play Some Chess!";
	}

public:

	enum class MoveType {
		NO_MOVE,
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

	enum class Direction {
		NONE,
		HORIZONTAL,
		VERTICAL,
		POSITIVE_DIAGONAL,
		NEGITIVE_DIAGONAL
	};

	enum class PieceType {
		PAWN	= 5,
		BISHOP	= 4,
		KNIGHT	= 3,
		ROOK	= 2,
		KING	= 1,
		QUEEN	= 0
	};

	enum class PieceColor {
		WHITE = 0,
		BLACK = 1
	};

	static void Log(std::string msg)
	{
		std::cout << msg << std::endl;
	}

	static int vtoi(olc::vi2d pos) { return pos.x + 8 * pos.y; }

	static bool bounded(int lower, int upper, int check) { return check >= lower && check <= upper; }
	static bool bounded(olc::vi2d lower, olc::vi2d upper, olc::vi2d check)
	{ return (check.x >= min(lower.x, upper.x) && check.y >= min(lower.y, upper.y)) &&
			 (check.x <= max(lower.x, upper.x) && check.y <= max(lower.y, upper.y)); }

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
		default:
			return "UNREACHABLE";
			break;
		}
	}

	static PieceColor getOpositeColor(PieceColor in) {
		static std::vector<PieceColor> colors{ PieceColor::BLACK, PieceColor::WHITE };
		return colors[(int)in];
	}

	struct Piece;
	struct GameBoard;
	struct PawnLogic;

	struct Move {
		olc::vi2d startPos, endPos;
		MoveType eType;
		Piece* secondaryPiece;

		Move(olc::vi2d startPosition, olc::vi2d endPosition, MoveType type) : startPos(startPosition), endPos(endPosition), eType(type), secondaryPiece(nullptr) {}
		Move(olc::vi2d startPosition, olc::vi2d endPosition, MoveType type, Piece* other) : startPos(startPosition), endPos(endPosition), eType(type), secondaryPiece(other) {}

		void drawSelf(Chess* pge) 
		{
			bool fixed = eType == MoveType::FIXED || eType == MoveType::FIXED_AND_ATTACK || eType == MoveType::DOUBLE_MOVE || eType == MoveType::EN_PASSANT;
			bool line = eType == MoveType::LINE || eType == MoveType::LINE_AND_ATTACK;
			bool attack = eType == MoveType::FIXED_AND_ATTACK || eType == MoveType::LINE_AND_ATTACK;

			if (fixed)
			{
				pge->FillRectDecal(endPos * 64 + olc::vi2d(3, 3), { 64, 64 }, MOVE_COLOR);

				if(attack) pge->FillRectDecal(endPos * 64 + olc::vi2d(3, 3), { 64, 64 }, ATTACK_COLOR);

				return;
			}

			if (line && pge->board.isStraight(startPos, endPos))
			{
				// Starts with the smallest of the x and y (the top left is 0,0) because the rectangle is rooted at the top left corner
				olc::vi2d topCorner = { min(startPos.x, endPos.x), min(startPos.y, endPos.y) };

				// The size is the displacement of the two positions
				olc::vi2d size = (startPos - endPos).abs();

				// If it is horizontal
				if (startPos.x != endPos.x)
				{
					size.y = 1;

					// If it extends rightwords, offset root so it does not overlap with the selected piece
					if (startPos.x < endPos.x)
					{
						topCorner.x += 1;

						// Check for line and attack, reduce size to make room for red square
						if (attack) size.x--;
					}

					// Else it is extending left, check for line and attack, move one closer and reduce size
					else if (attack)
					{
						topCorner.x++;
						size.x--;
					}
				}

				// If it is vertical
				else if (startPos.y != endPos.y)
				{
					size.x = 1;

					// If it extends downwords, offset root so it does not overlap with the selected piece
					if (startPos.y < endPos.y)
					{
						topCorner.y += 1;

						// Check for line and attack, reduce size to make room for red square
						if (attack) size.y--;
					}

					// Else it is extending upwords, check for line and attack, move one closer and reduce size
					else if (attack)
					{
						topCorner.y++;
						size.y--;
					}
				}

				pge->FillRectDecal( topCorner * 64 + olc::vi2d(3, 3), size * 64, MOVE_COLOR );
				
				if(attack)
					pge->FillRectDecal(endPos * 64 + olc::vi2d(3, 3), {64, 64}, ATTACK_COLOR);

				return;

				// Alternative
				/*
				olc::vi2d pos = olc::vi2d{
						min(startPos.x, endPos.x) + (startPos.x != endPos.x && startPos.x < endPos.x ? 1 : 0),
						min(startPos.y, endPos.y) + (startPos.y != endPos.y && startPos.y < endPos.y ? 1 : 0) };

				olc::vi2d size = (startPos - endPos).abs();

				size.x += (startPos.x != endPos.x ? 0 : 1);
				size.y += (startPos.y != endPos.y ? 0 : 1);

				pge->FillRectDecal(
					pos * 64 + olc::vi2d(3, 3),
					size * 64,
					MOVE_COLOR
				);

				pge->FillRectDecal(
					olc::vi2d{
						min(startPos.x, endPos.x) + (startPos.x != endPos.x && startPos.x < endPos.x ? 1 : 0),
						min(startPos.y, endPos.y) + (startPos.y != endPos.y && startPos.y < endPos.y ? 1 : 0) } * 64 + olc::vi2d(3, 3),
					((startPos - endPos).abs() + olc::vi2d{
						(startPos.x != endPos.x ? 0 : 1),
						(startPos.y != endPos.y ? 0 : 1) }) * 64,
					MOVE_COLOR
				);
				*/
			}

			else if (line && pge->board.isDiagonal(startPos, endPos))
			{
				olc::vi2d diff = endPos - startPos, 
						  slope = diff / diff.abs();

				for (olc::vi2d pos = startPos + slope; pos != endPos; pos += slope)
					pge->FillRectDecal(pos * 64 + olc::vi2d{ 3, 3 }, { 64, 64 }, MOVE_COLOR);

				if (attack) pge->FillRectDecal(endPos * 64 + olc::vi2d{ 3, 3 }, { 64, 64 }, ATTACK_COLOR);
				else		pge->FillRectDecal(endPos * 64 + olc::vi2d{ 3, 3 }, { 64, 64 }, MOVE_COLOR);
			}
		}
	};

	struct kingState {
		Piece* ptr;
		std::set<olc::vi2d> validMoves;
		bool check;

		void fillKingMap()
		{
			validMoves.clear();

			olc::vi2d pos = ptr->pos;

			// TODO: Not the best... I'm tired ok...
			validMoves.emplace(pos + olc::vi2d{ 1, 1 });
			validMoves.emplace(pos + olc::vi2d{ 0, 1 });
			validMoves.emplace(pos + olc::vi2d{ -1, 1 });
			validMoves.emplace(pos + olc::vi2d{ -1, 0 });
			validMoves.emplace(pos + olc::vi2d{ -1,-1 });
			validMoves.emplace(pos + olc::vi2d{ 0,-1 });
			validMoves.emplace(pos + olc::vi2d{ 1,-1 });
			validMoves.emplace(pos + olc::vi2d{ 1, 0 });
		}
	};

	struct PieceLogic {
	public:
		bool firstMove = false, justDoubleMoved = false;
		int moveCount = 0;

	private: 

		enum class FixedMoveFlags : int {
			NO_FLAG			= 0,
			ATTACK_ONLY		= 1,
			MOVE_ONLY		= 2,
			FIRST_MOVE_ONLY = 3,
			EN_PASSANT		= 4,
			DOUBLE_MOVE		= 5
		};

		olc::vi2d NO_FIXED_FLAGS = { (int)FixedMoveFlags::NO_FLAG, (int)FixedMoveFlags::NO_FLAG };

		// Gets the apropriate move for a single liniar section
		void appendSegmentMove(std::vector<Move>& moves, Piece* piece, olc::vi2d end, bool isBordering, GameBoard& board, bool debug, int quadrent)
		{
			// Never meets any piece in given quadrent
			if (end == piece->pos || (board.isOnAnyBoarder(end) && !board.getPieceAt(end)))
			{
				if (debug) Log("  No encounter in quadrent: " + std::to_string(quadrent));
				if (isBordering)
				{
					if (debug) Log("    NOT BORDERING");
					moves.push_back(Move(piece->pos, end, MoveType::LINE));
				}
			}
			// Meets ally at position end in given quadrent
			else if (board.getPieceAt(end)->eColor != piece->eEnemyColor)
			{
				// Offset end position by one to avoid overlap
				olc::vi2d diff = end - piece->pos,
					adjustedPos = end - olc::vi2d {
						diff.x ? (diff.x / std::abs(diff.x)) : 0,
						diff.y ? (diff.y / std::abs(diff.y)) : 0
					};

				if (debug) Log("  Ally in quadrent " + std::to_string(quadrent) + ": " + end.str());

				if (adjustedPos != piece->pos)
				{
					if (debug) Log("    NOT COLOCATED");
					moves.push_back(Move(piece->pos, adjustedPos, MoveType::LINE));
				}
			}
			// Meets enemy at position end in given quadrent
			else if (board.getPieceAt(end)->eColor == piece->eEnemyColor)
			{
				if (debug) Log("  Enemy in quadrent " + std::to_string(quadrent) + ": " + end.str());
				moves.push_back(Move(piece->pos, end, MoveType::LINE_AND_ATTACK, board.getPieceAt(end)));
			}
		}

		// Run logic for a generic cross move type
		void appendCrossMoves(std::vector<Move>& moves, Piece* piece, Game::vb2dPair bordered, GameBoard& board, bool debug)
		{
			Game::vi2dPair horizontalEnds = board.findOnLine(piece->pos, { +1, 0 }),
						   verticalEnds = board.findOnLine(piece->pos, { 0,-1 });

			appendSegmentMove(moves, piece, horizontalEnds.a, !bordered.a.y, board, debug, 1);
			appendSegmentMove(moves, piece, horizontalEnds.b, !bordered.a.x, board, debug, 2);
			appendSegmentMove(moves, piece, verticalEnds.a, !bordered.b.y, board, debug, 3);
			appendSegmentMove(moves, piece, verticalEnds.b, !bordered.b.x, board, debug, 4);

			if (debug) Log("  Horizontal: " + horizontalEnds.a.str() + ", " + horizontalEnds.b.str() + " Vertical: " + verticalEnds.a.str() + ", " + verticalEnds.b.str() +
				" Moves: " + std::to_string(moves.size()));
		}

		// Run logic for a generic X move type
		void appendDiagonalMoves(std::vector<Move>& moves, Piece* piece, Game::vb2dPair bordered, GameBoard& board, bool debug)
		{
			Game::vi2dPair positiveDiagonalEnds = board.findOnLine(piece->pos, {+1,-1}),
						   negitiveDiagonalEnds = board.findOnLine(piece->pos, {-1,-1});

			appendSegmentMove(moves, piece, positiveDiagonalEnds.a, !bordered.a.y && !bordered.b.y, board, debug, 1);
			appendSegmentMove(moves, piece, positiveDiagonalEnds.b, !bordered.a.x && !bordered.b.x, board, debug, 2);
			appendSegmentMove(moves, piece, negitiveDiagonalEnds.a, !bordered.a.x && !bordered.b.y, board, debug, 3);
			appendSegmentMove(moves, piece, negitiveDiagonalEnds.b, !bordered.a.y && !bordered.b.x, board, debug, 4);

			if (debug)
				Log(
					"  Positive diagonal: " + positiveDiagonalEnds.a.str() + ", " + positiveDiagonalEnds.b.str() +
					" Negitive diagonal: " + negitiveDiagonalEnds.a.str() + ", " + negitiveDiagonalEnds.b.str() +
					" Boardering: (l,r) " + bordered.a.str() + ", (b,t)" + bordered.b.str() +
					" Moves: " + std::to_string(moves.size()));
		}

		void appendFixedMoves(std::vector<Move>& moves, Piece* piece, GameBoard& board, std::vector<Game::vi2dPair> tryPositions, bool debug)
		{
			if (debug) Log(" Append fixed moves:");

			for (Game::vi2dPair pair : tryPositions)
			{
				// Get position
				olc::vi2d tryPos = pair.a;

				// Ensure position is bounded
				if (!board.boundedInMap(tryPos))
				{
					if (debug) Log("  XXXX  Pos: " + tryPos.str() + " =OUT OF BOUNDS=");
					continue;
				}

				// Get logic flags
				bool attackOnly		= pair.b.x == (int) FixedMoveFlags::ATTACK_ONLY		|| pair.b.y == (int) FixedMoveFlags::ATTACK_ONLY,
					 moveOnly		= pair.b.x == (int) FixedMoveFlags::MOVE_ONLY		|| pair.b.y == (int) FixedMoveFlags::MOVE_ONLY,
					 firstMoveOnly	= pair.b.x == (int) FixedMoveFlags::FIRST_MOVE_ONLY	|| pair.b.y == (int) FixedMoveFlags::FIRST_MOVE_ONLY,
					 enPassant		= pair.b.x == (int) FixedMoveFlags::EN_PASSANT		|| pair.b.y == (int) FixedMoveFlags::EN_PASSANT,
					 doubleMove		= pair.b.x == (int) FixedMoveFlags::DOUBLE_MOVE		|| pair.b.y == (int) FixedMoveFlags::DOUBLE_MOVE;

				// Ensure flags do not contradict
				if (attackOnly && moveOnly)
				{
					if (debug) Log("  XXXX  Pos: " + tryPos.str() + " =ATTK & MOV: CONTRADITORY FLAGS=");
					continue;
				}

				if ((firstMoveOnly || doubleMove) && !firstMove)
				{
					if (debug) Log("  XXXX  Pos: " + tryPos.str() + " =FST MOV: NOT FIRST MOVE=");
					continue;
				}

				if (enPassant && moveCount < 3)
				{
					if (debug) Log("  XXXX  Pos: " + tryPos.str() + " =EN PASNT: LESS THAN 3 MOVES=");
					continue;
				}

				// Fetch piece from position
				Piece* pieceAtPos = board.getPieceAt(tryPos);
				
				if (debug) Log("  Pos: " + tryPos.str() +
					" Flags: " + 
					(attackOnly		? "ATTACK_ONLY " : "") + 
					(moveOnly		? "MOVE_ONLY " : "") + 
					(firstMoveOnly	? "FIRST_MOVE_ONLY " : "") +
					(enPassant		? "EN_PASSANT " : "")
				);

				if (enPassant && !pieceAtPos)
				{
					Piece* enemy = board.getPieceAt(tryPos - olc::vi2d{ 0, piece->direction });

					if (!enemy || enemy->eColor == piece->eColor || enemy->eType != PieceType::PAWN || !enemy->logic.justDoubleMoved)
						continue;

					moves.push_back(Move(piece->pos, tryPos, MoveType::EN_PASSANT, enemy));

				}

				// Movement check
				if (!attackOnly && !pieceAtPos)
					moves.push_back(Move(piece->pos, tryPos, doubleMove ? MoveType::DOUBLE_MOVE : MoveType::FIXED));

				// Attack check
				if (!moveOnly && pieceAtPos && pieceAtPos->eColor == piece->eEnemyColor)
				{
					if (pieceAtPos->eType == PieceType::KING) board.setCheck(piece->eEnemyColor);
					moves.push_back(Move(piece->pos, tryPos, MoveType::FIXED_AND_ATTACK, pieceAtPos));
				}
			}
		}

	private:
		// En pasont, rank up
		std::vector<Move> pawnLogic(Piece* piece, GameBoard& board) 
		{
			std::vector<Move> moves;
			bool debug = Debug::DebugPieceLogic & Debug::Pawn;

			if (debug) Log("Color: " + std::to_string((int)piece->eColor) + " Pos: " + piece->pos.str());

			appendFixedMoves(moves, piece, board, {
				// Single move
				{{ piece->pos.x    , piece->pos.y + piece->direction     }, { (int)FixedMoveFlags::MOVE_ONLY,   (int)FixedMoveFlags::NO_FLAG }},
				{{ piece->pos.x    , piece->pos.y + piece->direction * 2 }, { (int)FixedMoveFlags::MOVE_ONLY,   (int)FixedMoveFlags::DOUBLE_MOVE }},
				// Fixed attacks
				{{ piece->pos.x - 1, piece->pos.y + piece->direction     }, { (int)FixedMoveFlags::ATTACK_ONLY, (int)FixedMoveFlags::NO_FLAG }},
				{{ piece->pos.x + 1, piece->pos.y + piece->direction     }, { (int)FixedMoveFlags::ATTACK_ONLY, (int)FixedMoveFlags::NO_FLAG }},
				// En passant
				{{ piece->pos.x + 1, piece->pos.y + piece->direction     }, { (int)FixedMoveFlags::EN_PASSANT,  (int)FixedMoveFlags::NO_FLAG }},
				{{ piece->pos.x - 1, piece->pos.y + piece->direction     }, { (int)FixedMoveFlags::EN_PASSANT,  (int)FixedMoveFlags::NO_FLAG }},
			}, debug);

			// Rank up if desired
			if (!board.boundedInMap({ piece->pos.x, piece->pos.y + piece->direction }))
				moves.push_back(Move(piece->pos, { piece->pos.x, piece->pos.y }, MoveType::RANK_UP));

			if (debug) Log(" Valid moves: ");

			if (debug) for (Move m : moves)
				Log("    TO: " + m.endPos.str() + " TYPE: " + moveToString(m.eType));

			if(debug) Log("");

			return moves;
		}

		std::vector<Move> bishopLogic(Piece* piece, GameBoard& board)
		{
			std::vector<Move> moves;
			Game::vb2dPair bordered = board.isPieceOnBoarder(piece->pos);
			bool debug = Debug::DebugPieceLogic & Debug::Bishop;

			if (debug) Log("Color: " + std::to_string((int)piece->eColor) + " Pos: " + piece->pos.str());

			appendDiagonalMoves(moves, piece, bordered, board, debug);

			if (debug) for (Move m : moves)
				Log("    TO: " + m.endPos.str() + " TYPE: " + moveToString(m.eType));

			if (debug) Log("");

			return moves;
		}

		std::vector<Move> knightLogic(Piece* piece, GameBoard& board)
		{
			std::vector<Move> moves;
			bool debug = (Debug::DebugPieceLogic & Debug::Knight) != 0;

			if (debug) Log("Color: " + std::to_string((int)piece->eColor) + " Pos: " + piece->pos.str());

			appendFixedMoves(moves, piece, board, {
				{{ piece->pos.x + 2, piece->pos.y + 1}, NO_FIXED_FLAGS},
				{{ piece->pos.x + 1, piece->pos.y + 2}, NO_FIXED_FLAGS},

				{{ piece->pos.x - 2, piece->pos.y + 1}, NO_FIXED_FLAGS},
				{{ piece->pos.x + 1, piece->pos.y - 2}, NO_FIXED_FLAGS},

				{{ piece->pos.x - 2, piece->pos.y - 1}, NO_FIXED_FLAGS},
				{{ piece->pos.x - 1, piece->pos.y - 2}, NO_FIXED_FLAGS},

				{{ piece->pos.x + 2, piece->pos.y - 1}, NO_FIXED_FLAGS},
				{{ piece->pos.x - 1, piece->pos.y + 2}, NO_FIXED_FLAGS},
			}, debug);

			if (debug) Log(" Valid moves: ");

			if (debug) for (Move m : moves)
				Log("    TO: " + m.endPos.str() + " TYPE: " + moveToString(m.eType));

			if (debug) Log("");

			return moves;
		}

		std::vector<Move> rookLogic(Piece* piece, GameBoard& board)
		{
			std::vector<Move> moves;
			Game::vb2dPair bordered = board.isPieceOnBoarder(piece->pos);
			bool debug = (Debug::DebugPieceLogic & Debug::Rook) != 0;

			if(debug) Log("Color: " + std::to_string((int)piece->eColor) + " Pos: " + piece->pos.str());

			appendCrossMoves(moves, piece, bordered, board, debug);

			if (debug) for (Move m : moves)
				Log("    TO: " + m.endPos.str() + " TYPE: " + moveToString(m.eType));

			if (debug) Log("");

			return moves;
		}

		std::vector<Move> kingLogic(Piece* piece, GameBoard& board, kingState& state)
		{
			std::vector<Move> moves;
			bool debug = (Debug::DebugPieceLogic & Debug::King) != 0;

			if (debug) Log("Color: " + std::to_string((int)piece->eColor) + " Pos: " + piece->pos.str());

			std::vector<Game::vi2dPair> tryMoves;

			for (olc::vi2d pos : state.validMoves) tryMoves.push_back(Game::vi2dPair{pos, NO_FIXED_FLAGS });

			appendFixedMoves(moves, piece, board, tryMoves /*{
				{{ piece->pos.x + 1, piece->pos.y + 1}, NO_FIXED_FLAGS},
				{{ piece->pos.x + 0, piece->pos.y + 1}, NO_FIXED_FLAGS},
				{{ piece->pos.x - 1, piece->pos.y + 1}, NO_FIXED_FLAGS},
				{{ piece->pos.x - 1, piece->pos.y + 0}, NO_FIXED_FLAGS},
				{{ piece->pos.x - 1, piece->pos.y - 1}, NO_FIXED_FLAGS},
				{{ piece->pos.x + 0, piece->pos.y - 1}, NO_FIXED_FLAGS},
				{{ piece->pos.x + 1, piece->pos.y - 1}, NO_FIXED_FLAGS},
				{{ piece->pos.x + 1, piece->pos.y + 0}, NO_FIXED_FLAGS},
			}*/, debug);

			if (debug) Log(" Valid moves: ");

			if (debug) for (Move m : moves)
				Log("    TO: " + m.endPos.str() + " TYPE: " + moveToString(m.eType));

			if (debug) Log("");
			
			return moves;
		}

		std::vector<Move> queenLogic(Piece* piece, GameBoard& board)
		{
			std::vector<Move> moves;
			Game::vb2dPair bordered = board.isPieceOnBoarder(piece->pos);
			bool debug = Debug::DebugPieceLogic & Debug::Queen;

			if (debug) Log("Color: " + std::to_string((int)piece->eColor) + " Pos: " + piece->pos.str());

			appendCrossMoves(moves, piece, bordered, board, debug);
			appendDiagonalMoves(moves, piece, bordered, board, debug);

			if (debug) for (Move m : moves)
				Log("    TO: " + m.endPos.str() + " TYPE: " + moveToString(m.eType));

			if (debug) Log("");

			return moves;
		}

	public:
		PieceLogic(PieceType type) : firstMove( true ) { }

		std::vector<Move> runLogic(Piece* piece, GameBoard& board)
		{
			switch (piece->eType)
			{
			case PieceType::PAWN:
				return pawnLogic(piece, board);
				break;
			case PieceType::BISHOP:
				return bishopLogic(piece, board);
				break;
			case PieceType::KNIGHT:
				return knightLogic(piece, board);
				break;
			case PieceType::ROOK:
				return rookLogic(piece, board);
				break;
			//case PieceType::KING:
			//	return kingLogic(piece, board);
			//	break;
			case PieceType::QUEEN:
				return queenLogic(piece, board);
				break;
			default:
				return std::vector<Move>();
			}
		}

		std::vector<Move> runLogic(Piece* piece, GameBoard& board, kingState& state)
		{
			if (piece->eType == PieceType::KING)
				kingLogic(piece, board, state);
		}
	};

	struct Piece {
		olc::vi2d pos; // Position of piece, top left is {0,0}
		std::vector<Move> moves; // Current move options
		PieceType eType; // Piece type
		PieceColor eColor, eEnemyColor;
		PieceLogic logic; // Underlying piece logic
		int direction; // Positive or negitive depending on color, determines forword
		bool displayMoves; // if the moves are displayed with colored squares

		Piece(PieceType type, PieceColor color)
			: eType{ type }, eColor{ color }, eEnemyColor{ getOpositeColor(color) },
			  pos{ 0,0 }, direction{ color == PieceColor::WHITE ? -1 : +1 },
			  logic(PieceLogic(type)), displayMoves{ false } {}

		Piece(PieceType type, PieceColor color, PieceLogic piecelogic)
			: eType{ type }, eColor{ color }, eEnemyColor{ getOpositeColor(color) },
			  pos{ 0,0 }, direction{ color == PieceColor::WHITE ? -1 : +1 },
			  logic{ piecelogic }, displayMoves{ false } {}

		Piece* operator= (Piece* rhs)
		{
			eType = rhs->eType;
			eColor = rhs->eColor;
			pos = {0,0};
			//moves = rhs->moves;
		}

		void drawSelf(Chess* pge) {
			pge->DrawDecal(pos * 64, pge->chessPieceSheet.decals[(int)eType][(int)eColor]);

			if (displayMoves)
			{
				for (Move m : moves)
					m.drawSelf(pge);
			}
		}

		bool tryMoveTo(GameBoard* board, olc::vi2d tryPos)
		{
			// TODO: make sure this is checked once
			if (tryPos == pos)
				return false;

			for (Move m : moves)
			{
				// Shortcut, if the end position is equal to the selected position
				if (m.endPos == tryPos)
				{
					// If there is a second piece effected
					if (m.secondaryPiece)
					{
						// If the move is an attack move, delete the piece in question
						if (m.eType != MoveType::CASTLE)
							board->deletePieceAt(m.secondaryPiece->pos);
					}

					// No longer first move
					logic.firstMove = false;
					logic.moveCount++;
					if (m.eType == MoveType::DOUBLE_MOVE) { logic.justDoubleMoved = true;  }
					else logic.justDoubleMoved = false;

					// Move to new location on the board and internally
					board->movePiece(pos, tryPos);
					pos = tryPos;

					// Update logic based on new position
					board->updateAllLogic();

					return true;
				}
				else if (m.eType == MoveType::LINE || m.eType == MoveType::LINE_AND_ATTACK)
				{
					if (!board->isLineBounded(m.startPos, m.endPos, tryPos))
						continue;

					// No longer first move
					logic.firstMove = false;
					logic.moveCount++;

					// Move to new location on the board and internally
					board->movePiece(pos, tryPos);
					pos = tryPos;

					// Update logic based on new position
					board->updateAllLogic();

					return true;
				}

			}

			return false;
		}

		void updLogic(GameBoard* board)
		{
			moves = logic.runLogic(this, *board);
		}
	};

	class GameBoard {
		const std::vector<Piece*> defaultBoard {
		//    A     B     C     D     E     F     G     H
			BROK, BKNT, BBSP, BQUN, BKNG, BBSP, BKNT, BROK, // 8
			BPWN, BPWN, BPWN, BPWN, BPWN, BPWN, BPWN, BPWN, // 7
			BLNK, BLNK, BLNK, BLNK, BLNK, BLNK, BLNK, BLNK, // 6
			BLNK, BLNK, BLNK, BLNK, BLNK, BLNK, BLNK, BLNK, // 5
			BLNK, BLNK, BLNK, BLNK, BLNK, BLNK, BLNK, BLNK, // 4
			BLNK, BLNK, BLNK, BLNK, BLNK, BLNK, BLNK, BLNK, // 3
			WPWN, WPWN, WPWN, WPWN, WPWN, WPWN, WPWN, WPWN, // 2
			WROK, WKNT, WBSP, WQUN, WKNG, WBSP, WKNT, WROK, // 1
		};

	public:
		std::vector<Piece*> board;
		olc::vi2d selectedPiece;
		bool isPieceSelected;
		PieceColor eColor;
		int turn;

		kingState whiteKing, blackKing;

	private:

	public:
		GameBoard()
		{
			board = defaultBoard;
			isPieceSelected = false;
			eColor = PieceColor::WHITE;

			for(int i = 0; i < 8; i++)
				for (int k = 0; k < 8; k++)
					if(board[vtoi({ k, i })])
						board[vtoi({ k, i })]->pos = { k, i };

			whiteKing.ptr = board[vtoi({ 4, 7 })];
			blackKing.ptr = board[vtoi({ 4, 0 })];
		}

		bool boundedInMap(olc::vi2d pos) { return pos.x >= 0 && pos.y >= 0 && pos.x < 8 && pos.y < 8; }
		Piece* getPieceAt(olc::vi2d pos) { return boundedInMap(pos) ? board[vtoi(pos)] : nullptr; }

		void setCheck(PieceColor color) {  }

		bool isOnAnyBoarder(olc::vi2d pos) { return pos.x == 0 || pos.x == 7 || pos.y == 0 || pos.y == 7; }
		// a = { left, right }, b = { bottom, top }
		Game::vb2dPair isPieceOnBoarder(olc::vi2d pos) { return { {pos.x == 0, pos.x == 7}, {pos.y == 7, pos.y == 0} }; }
		bool isPieceAtBounded(olc::vi2d pos) { return boundedInMap(pos) && board[vtoi(pos)]; }
		bool isPieceAtBounded(olc::vi2d pos, PieceColor color) { Piece* p = getPieceAt(pos); return p && p->eColor == color; }
		bool isPieceAt(olc::vi2d pos) { return board[vtoi(pos)]; }
		bool isPieceAt(olc::vi2d pos, PieceColor color) { return board[vtoi(pos)]->eColor == color; }

		void deletePieceAt(olc::vi2d pos) { if(boundedInMap(pos)) board[vtoi(pos)] = nullptr; }
		void movePiece(olc::vi2d from, olc::vi2d to) { if (getPieceAt(from) && !getPieceAt(to)) { board[vtoi(to)] = getPieceAt(from); deletePieceAt(from); } }
		void updateAllLogic()
		{
			whiteKing.fillKingMap();
			blackKing.fillKingMap();

			for (Piece* p : board) if (p && p->eType != PieceType::KING) { p->updLogic(this); }

			if (whiteKing.validMoves.empty()) Log("CHECKMATE! BLACK WINS!");
			else { for (olc::vi2d pos : whiteKing.validMoves) {  } }

			if (blackKing.validMoves.empty()) Log("CHECKMATE! WHITE WINS!");

			if (Debug::DebugPieceLogic != 0) Log("");
		}

		// Horizont = {+1, 0} : a = Right, b = Left
		// Vertical = { 0,-1} : a = Up, b = Down
		// Pos diag = {+1,-1} : a = Top, b = Bottom
		// Neg diag = {-1,-1} : a = Top, b = Bottom
		// a = negitive end, b = positive end
		Game::vi2dPair findOnLine(olc::vi2d start, olc::vi2d slope)
		{
			bool endA = false, endB = false;
			Game::vi2dPair out;

			for (int i = 1; !endA || !endB; i++)
			{
				// Runs into a piece, vector becomes that position
				if (!endA)
				{
					olc::vi2d newPos = { start.x + (i * slope.x), start.y + (i * slope.y) };

					if (!boundedInMap(newPos))
					{
						out.a = newPos - slope;
						endA = true;
					}
					else if (isPieceAt(newPos))
					{
						out.a = newPos;
						endA = true;
					}
				}

				if (!endB)
				{
					olc::vi2d newPos = { start.x - (i * slope.x), start.y - (i * slope.y) };

					if (!boundedInMap(newPos))
					{
						out.b = newPos + slope;
						endB = true;
					}
					else if (isPieceAt(newPos))
					{
						out.b = newPos;
						endB = true;
					}
				}
			}

			return out;
		}

		bool isHorizontal(olc::vi2d pos1, olc::vi2d pos2) { return pos1.y == pos2.y && pos1.x != pos2.x; }
		bool isVertical(olc::vi2d pos1, olc::vi2d pos2) { return pos1.x == pos2.x && pos1.y != pos2.y; }
		bool isStraight(olc::vi2d pos1, olc::vi2d pos2) { return isHorizontal(pos1, pos2) || isVertical(pos1, pos2); }

		bool isPositiveDiagonal(olc::vi2d pos1, olc::vi2d pos2) { return (pos1 - pos2).x == -(pos1 - pos2).y; }
		bool isNegitiveDiagonal(olc::vi2d pos1, olc::vi2d pos2) { return (pos1 - pos2).x ==  (pos1 - pos2).y; }
		bool isDiagonal(olc::vi2d pos1, olc::vi2d pos2) { return isPositiveDiagonal(pos1, pos2) || isNegitiveDiagonal(pos1, pos2); }

		std::pair<bool, Direction> getLineBoundedInfo(olc::vi2d origin, olc::vi2d limit, olc::vi2d check)
		{
			if (isHorizontal(origin, limit))
			{
				if(isHorizontal(origin, check) && bounded(min(origin.x, limit.x), max(origin.x, limit.x), check.x))
					return std::pair<bool, Direction>(true, Direction::HORIZONTAL);
				else
					return std::pair<bool, Direction>(false, Direction::HORIZONTAL);
			}

			else if (isVertical(origin, limit))
			{
				if (isVertical(origin, check) && bounded(min(origin.y, limit.y), max(origin.y, limit.y), check.y))
					return std::pair<bool, Direction>(true, Direction::VERTICAL);
				else
					return std::pair<bool, Direction>(false, Direction::VERTICAL);
			}

			else if (isPositiveDiagonal(origin, limit))
			{
				if (isPositiveDiagonal(origin, check) && bounded(min(origin, limit), max(origin, limit), check))
					return std::pair<bool, Direction>(true, Direction::POSITIVE_DIAGONAL);
				else
					return std::pair<bool, Direction>(false, Direction::POSITIVE_DIAGONAL);
			}

			else if (isHorizontal(origin, limit))
			{
				if (isHorizontal(origin, check) && bounded(min(origin, limit), max(origin, limit), check))
					return std::pair<bool, Direction>(true, Direction::NEGITIVE_DIAGONAL);
				else
					return std::pair<bool, Direction>(false, Direction::NEGITIVE_DIAGONAL);
			}
		}

		bool isLineBounded(olc::vi2d origin, olc::vi2d limit, olc::vi2d check)
		{
			return (isHorizontal(origin, limit) && isHorizontal(origin, check) && bounded(min(origin.x, limit.x), max(origin.x, limit.x), check.x)) ||
				(isVertical(origin, limit) && isVertical(origin, check) && bounded(min(origin.y, limit.y), max(origin.y, limit.y), check.y)) || 
				(isPositiveDiagonal(origin, limit) && isPositiveDiagonal(origin, check) && bounded(min(origin, limit), max(origin, limit), check)) ||
				(isNegitiveDiagonal(origin, limit) && isNegitiveDiagonal(origin, check) && bounded(min(origin, limit), max(origin, limit), check));
		}

		olc::vi2d screenToBoard(olc::vi2d pos) { return { pos.x / 64, pos.y / 64 }; }

		void checkInput(Chess* pge)
		{
			if (pge->GetMouse(0).bPressed)
			{
				olc::vi2d pos = screenToBoard(pge->GetMousePos());

				// If the selection is out of range, return
				if (!boundedInMap(pos))
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
						eColor = eColor == PieceColor::WHITE ? PieceColor::BLACK : PieceColor::WHITE;
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

		//Debug::DebugPieceLogic |= Debug::Pawn;

		board.updateAllLogic();

		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
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