#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"


#define WPWN new Piece(PAWN, WHITE, PieceLogic(PAWN))
#define WROK new Piece(ROOK, WHITE, PieceLogic(ROOK))
#define WKNT new Piece(KNIGHT, WHITE)
#define WBSP new Piece(BISHOP, WHITE, PieceLogic(BISHOP))
#define WQUN new Piece(QUEEN, WHITE)
#define WKNG new Piece(KING, WHITE)

#define BPWN new Piece(PAWN, BLACK, PieceLogic(PAWN))
#define BROK new Piece(ROOK, BLACK, PieceLogic(ROOK))
#define BKNT new Piece(KNIGHT, BLACK)
#define BBSP new Piece(BISHOP, BLACK, PieceLogic(BISHOP))
#define BQUN new Piece(QUEEN, BLACK)
#define BKNG new Piece(KING, BLACK)

#define NONE nullptr

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
}

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

	static int vtoi(olc::vi2d pos) { return pos.x + 8 * pos.y; }

	static bool bounded(int lower, int upper, int check) { return check >= lower && check <= upper; }
	static bool bounded(olc::vi2d lower, olc::vi2d upper, olc::vi2d check)
	{ return (check.x >= min(lower.x, upper.x) && check.y >= min(lower.y, upper.y)) &&
			 (check.x <= max(lower.x, upper.x) && check.y <= max(lower.y, upper.y)); }

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
		default:
			return "UNREACHABLE";
			break;
		}
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

	static PieceColor getOpositeColor(PieceColor in) {
		static std::vector<PieceColor> colors{ BLACK, WHITE };
		return colors[in];
	}

	struct PieceLogic {
	public:
		bool firstMove;
		PieceType eType;

	private:

		int getQuadrent(olc::vi2d slope)
		{
			if (slope == olc::vi2d{ 1, 1 })
				return 1;
			else if (slope == olc::vi2d{ -1, 1 })
				return 2;
			else if (slope == olc::vi2d{ -1, -1 })
				return 3;
			else if (slope == olc::vi2d{ 1, -1 })
				return 4;
		}

		Move* checkLineMove(Piece* piece, olc::vi2d end, bool isBordering, GameBoard& board, bool debug, int quadrent)
		{
			// Never meets any piece in given quadrent
			if (end == piece->pos || (board.isOnAnyBoarder(end) && !board.getPieceAt(end)))
			{
				if (debug) Log("  No enemy in quadrent " + std::to_string(quadrent));
				if (isBordering)
				{
					if (debug) Log("    NOT BORDERING");
					return new Move(piece->pos, end, MoveType::LINE);
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
					return new Move(piece->pos, adjustedPos, MoveType::LINE);
				}
			}
			// Meets enemy at position end in given quadrent
			else if (board.getPieceAt(end)->eColor == piece->eEnemyColor)
			{
				if (debug) Log("  Enemy in quadrent " + std::to_string(quadrent) + ": " + end.str());
				return new Move(piece->pos, end, MoveType::LINE_AND_ATTACK, board.getPieceAt(end));
			}

			return nullptr;
		}

		std::vector<Move> lineLogic(Piece* piece, GameBoard& board, bool debug)
		{
			std::vector<Move> moves;
			Game::vb2dPair bordered = board.isPieceOnBoarder(piece->pos);

			// ==== HORIZONTAL ==== //
			Game::vi2dPair horizontalEnds = board.findOnHorizontal(piece->pos);

			if (auto move = checkLineMove(piece, horizontalEnds.a, !bordered.a.y, board, debug, 1)) moves.push_back(*move);
			if (auto move = checkLineMove(piece, horizontalEnds.b, !bordered.a.x, board, debug, 2)) moves.push_back(*move);
			
			// ==== VERTICAL ==== //
			Game::vi2dPair verticalEnds = board.findOnVertical(piece->pos);

			if (auto move = checkLineMove(piece, verticalEnds.a, !bordered.b.y, board, debug, 3)) moves.push_back(*move);
			if (auto move = checkLineMove(piece, verticalEnds.b, !bordered.b.x, board, debug, 4)) moves.push_back(*move);

			if (debug) Log("  Horizontal: " + horizontalEnds.a.str() + ", " + horizontalEnds.b.str() + " Vertical: " + verticalEnds.a.str() + ", " + verticalEnds.b.str() +
				" Moves: " + std::to_string(moves.size()));

			return moves;
		}

		Move* checkDiagonalMove(Piece* piece, olc::vi2d end, bool isBordering, GameBoard& board, bool debug, int quadrent)
		{
			// Never meets any piece in given quadrent
			if (end == piece->pos || (board.isOnAnyBoarder(end) && !board.getPieceAt(end)))
			{
				if (debug) Log("  No enemy in quadrent " + std::to_string(quadrent));
				if (isBordering)
				{
					if (debug) Log("    NOT BORDERING");
					return new Move(piece->pos, end, MoveType::LINE);
				}
			}
			// Meets ally at position end in given quadrent
			else if (board.getPieceAt(end)->eColor != piece->eEnemyColor)
			{
				olc::vi2d diff = end - piece->pos,
				slope = diff / diff.abs();

				olc::vi2d adjustedPos = end - slope;

				if (debug) Log("  Ally in quadrent " + std::to_string(quadrent) + ": " + end.str());
				if (adjustedPos != piece->pos)
				{
					if (debug) Log("    NOT COLOCATED");
					return new Move(piece->pos, adjustedPos, MoveType::LINE);
				}
			}
			// Meets enemy at position end in given quadrent
			else if (board.getPieceAt(end)->eColor == piece->eEnemyColor)
			{
				if (debug) Log("  Enemy in quadrent " + std::to_string(quadrent) + ": " + end.str());
				return new Move(piece->pos, end, MoveType::LINE_AND_ATTACK, board.getPieceAt(end));
			}

			return nullptr;
		}

		std::vector<Move> diagonalLogic(Piece* piece, GameBoard& board, bool debug)
		{
			std::vector<Move> moves;
			Game::vb2dPair bordered = board.isPieceOnBoarder(piece->pos);

			// ==== HORIZONTAL ==== //
			Game::vi2dPair positiveDiagonalEnds = board.findOnPositiveDiagonal(piece->pos);

			if (auto move = checkDiagonalMove(piece, positiveDiagonalEnds.a, !bordered.a.y && !bordered.b.y, board, debug, 1)) moves.push_back(*move);
			if (auto move = checkDiagonalMove(piece, positiveDiagonalEnds.b, !bordered.a.x && !bordered.b.x, board, debug, 2)) moves.push_back(*move);

			// ==== VERTICAL ==== //
			Game::vi2dPair negitiveDiagonalEnds = board.findOnNegitiveDiagonal(piece->pos);

			if (auto move = checkDiagonalMove(piece, negitiveDiagonalEnds.a, !bordered.a.x && !bordered.b.y, board, debug, 3)) moves.push_back(*move);
			if (auto move = checkDiagonalMove(piece, negitiveDiagonalEnds.b, !bordered.a.y && !bordered.b.x, board, debug, 4)) moves.push_back(*move);

			if (debug)
				Log(
					"  Positive diagonal: " + positiveDiagonalEnds.a.str() + ", " + positiveDiagonalEnds.b.str() +
					" Negitive diagonal: " + negitiveDiagonalEnds.a.str() + ", " + negitiveDiagonalEnds.b.str() +
					" Boardering: (l,r) " + bordered.a.str() + ", (b,t)" + bordered.b.str() +
					" Moves: " + std::to_string(moves.size()));

			return moves;
		}

		// En pasont, rank up
		std::vector<Move> pawnLogic(Piece* piece, GameBoard& board) 
		{
			std::vector<Move> moves;
			bool debug = Debug::DebugPieceLogic & Debug::Pawn;

			// Single move forword - Must have one empty space
			if (board.boundedInMap({ piece->pos.x, piece->pos.y + piece->direction }) && !board.isPieceAt({ piece->pos.x, piece->pos.y + piece->direction }))
			{
				moves.push_back(Move(piece->pos, { piece->pos.x, piece->pos.y + piece->direction }, MoveType::FIXED));

				// Double move forword - Must have one additional empty space
				if (firstMove && !board.isPieceAtBounded({ piece->pos.x, piece->pos.y + 2 * piece->direction }))
					moves.push_back(Move(piece->pos, { piece->pos.x, piece->pos.y + 2 * piece->direction }, MoveType::FIXED));
			}

			// Attack left
			if (board.isPieceAtBounded({ piece->pos.x - 1, piece->pos.y + piece->direction }, piece->eEnemyColor))
				moves.push_back(Move(piece->pos, { piece->pos.x - 1, piece->pos.y + piece->direction }, MoveType::FIXED_AND_ATTACK, board.getPieceAt({ piece->pos.x - 1, piece->pos.y + piece->direction })));
			// Attack right
			if (board.isPieceAtBounded({ piece->pos.x + 1, piece->pos.y + piece->direction }, piece->eEnemyColor))
				moves.push_back(Move(piece->pos, { piece->pos.x + 1, piece->pos.y + piece->direction }, MoveType::FIXED_AND_ATTACK, board.getPieceAt({ piece->pos.x + 1, piece->pos.y + piece->direction })));

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
			if (!board.boundedInMap({ piece->pos.x, piece->pos.y + piece->direction }))
				moves.push_back(Move(piece->pos, { piece->pos.x, piece->pos.y }, MoveType::RANK_UP));
			/*
			Log("Moves: " + std::to_string(moves.size()));

			for (Move m : moves)
				Log("  " + moveToString(m.eType));
*/
			if (debug) Log("");

			return moves;
		}

		// DONE
		std::vector<Move> bishopLogic(Piece* piece, GameBoard& board)
		{
			std::vector<Move> moves;
			Game::vb2dPair bordered = board.isPieceOnBoarder(piece->pos);
			bool debug = Debug::DebugPieceLogic & Debug::Bishop;

			if (debug) Log("Color: " + std::to_string(piece->eColor) + " Pos: " + piece->pos.str());

			moves = diagonalLogic(piece, board, debug);

			if (debug) for (Move m : moves)
				Log("    TO: " + m.endPos.str() + " TYPE: " + moveToString(m.eType));

			if (debug) Log("");

			return moves;
		}

		std::vector<Move> knightLogic(Piece* piece, GameBoard& board)
		{
			std::vector<Move> moves;
			olc::vi2d pos = piece->pos;
			int forword = piece->eColor == PieceColor::WHITE ? -1 : +1;
			PieceColor enemy = getOpositeColor(piece->eColor);

			return moves;
		}

		// DONE
		std::vector<Move> rookLogic(Piece* piece, GameBoard& board)
		{
			std::vector<Move> moves;
			Game::vb2dPair bordered = board.isPieceOnBoarder(piece->pos);
			bool debug = (Debug::DebugPieceLogic & Debug::Rook) != 0;

			if(debug) Log("Color: " + std::to_string(piece->eColor) + " Pos: " + piece->pos.str());

			moves = lineLogic(piece, board, debug);

			if (debug) for (Move m : moves)
				Log("    TO: " + m.endPos.str() + " TYPE: " + moveToString(m.eType));

			if (debug) Log("");

			return moves;
		}

		std::vector<Move> kingLogic(Piece* piece, GameBoard& board)
		{
			std::vector<Move> moves;
			olc::vi2d pos = piece->pos;
			int forword = piece->eColor == PieceColor::WHITE ? -1 : +1;
			PieceColor enemy = getOpositeColor(piece->eColor);

			return moves;
		}

		// DONE
		std::vector<Move> queenLogic(Piece* piece, GameBoard& board)
		{
			std::vector<Move> moves;
			Game::vb2dPair bordered = board.isPieceOnBoarder(piece->pos);
			bool debug = Debug::DebugPieceLogic & Debug::Queen;

			if (debug) Log("Color: " + std::to_string(piece->eColor) + " Pos: " + piece->pos.str());

			moves = lineLogic(piece, board, debug);

			std::vector<Move> diagonalMoves = diagonalLogic(piece, board, debug);
			moves.insert(moves.end(), diagonalMoves.begin(), diagonalMoves.end());

			if (debug) for (Move m : moves)
				Log("    TO: " + m.endPos.str() + " TYPE: " + moveToString(m.eType));

			if (debug) Log("");

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
				return bishopLogic(piece, board);
				break;
			case KNIGHT:
				return knightLogic(piece, board);
				break;
			case ROOK:
				return rookLogic(piece, board);
				break;
			case KING:
				return kingLogic(piece, board);
				break;
			case QUEEN:
				return queenLogic(piece, board);
				break;
			default:
				return std::vector<Move>();
			}
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
			pge->DrawDecal(pos * 64, pge->chessPieceSheet.decals[eType][eColor]);

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
				else if (m.eType == MoveType::LINE || m.eType == MoveType::LINE_AND_ATTACK)
				{
					bool moveHorizontal = board->isHorizontalBounded(m.startPos, m.endPos, tryPos);
					bool moveVertical = board->isVerticalBounded(m.startPos, m.endPos, tryPos);
					bool movePosDiag = board->isPositiveDiagonalBounded(m.startPos, m.endPos, tryPos);
					bool moveNegDiag = board->isNegitiveDiagonalBounded(m.startPos, m.endPos, tryPos);

					//	{ return isPositiveDiagonal(origin, limit) && isPositiveDiagonal(origin, check) && bouded(min(origin, limit), max(origin, limit), check); }

					Log("Start: " + m.startPos.str() + " End: " + m.endPos.str() + " Try: " + tryPos.str());
					Log("Is diag: " + std::to_string(board->isNegitiveDiagonal(m.startPos, m.endPos)) + " " + std::to_string(board->isNegitiveDiagonal(m.startPos, tryPos)));
					Log("Pos: " + std::to_string(movePosDiag) + " Neg: " + std::to_string(moveNegDiag));

					// Either x or y is flat (horizontal or vertical), or x and y have a slope of 1 (diagonals)
					if (!(moveHorizontal || moveVertical || movePosDiag || moveNegDiag))
						continue;

					// No longer first move
					logic.firstMove = false;

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
		}

		bool boundedInMap(olc::vi2d pos) { return pos.x >= 0 && pos.y >= 0 && pos.x < 8 && pos.y < 8; }
		Piece* getPieceAt(olc::vi2d pos) { return boundedInMap(pos) ? board[vtoi(pos)] : nullptr; }

		bool isOnAnyBoarder(olc::vi2d pos) { return pos.x == 0 || pos.x == 7 || pos.y == 0 || pos.y == 7; }
		// a = { left, right }, b = { bottom, top }
		Game::vb2dPair isPieceOnBoarder(olc::vi2d pos) { return { {pos.x == 0, pos.x == 7}, {pos.y == 7, pos.y == 0} }; }
		bool isPieceAtBounded(olc::vi2d pos) { return boundedInMap(pos) && board[vtoi(pos)]; }
		bool isPieceAtBounded(olc::vi2d pos, PieceColor color) { Piece* p = getPieceAt(pos); return p && p->eColor == color; }
		bool isPieceAt(olc::vi2d pos) { return board[vtoi(pos)]; }
		bool isPieceAt(olc::vi2d pos, PieceColor color) { return board[vtoi(pos)]->eColor == color; }

		void deletePieceAt(olc::vi2d pos) { if(boundedInMap(pos)) board[vtoi(pos)] = nullptr; }
		void movePiece(olc::vi2d from, olc::vi2d to) { if (getPieceAt(from) && !getPieceAt(to)) { board[vtoi(to)] = getPieceAt(from); deletePieceAt(from); } }
		void updateAllLogic() { for (Piece* p : board) if (p) { p->updLogic(this); } if (Debug::DebugPieceLogic != 0) Log(""); }

		// a = Right, b = Left
		Game::vi2dPair findOnHorizontal(olc::vi2d start)
		{
			bool endR = false, endL = false;
			Game::vi2dPair out;
			out.a = { -1,-1 };
			out.b = { -1,-1 };

			for (int i = 1; !endR || !endL; i++)
			{
				// Never runs into a piece, vector stays as -1, -1
				if (!endR && !boundedInMap({ start.x + i, start.y }))
				{
					out.a = { start.x + (i - 1), start.y };
					endR = true;
				}
				// Runs into a piece, vector becomes that position
				else if (!endR && isPieceAt({ start.x + i, start.y }))
				{
					out.a = { start.x + i, start.y };
					endR = true;
				}

				// Never runs into a piece, vector stays as -1, -1
				if (!endL && !boundedInMap({ start.x - i, start.y }))
				{
					out.b = { start.x - (i - 1), start.y };
					endL = true;
				}
				// Runs into a piece, vector becomes that position
				else if (!endL && isPieceAt({ start.x - i, start.y }))
				{
					out.b = { start.x - i, start.y };
					endL = true;
				}
			}

			return out;
		}

		// a = Up, b = Down
		Game::vi2dPair findOnVertical(olc::vi2d start)
		{
			bool endD = false, endU = false;
			Game::vi2dPair out;
			out.a = { -1,-1 };
			out.b = { -1,-1 };

			for (int i = 1; !endD || !endU; i++)
			{
				// Never runs into a piece, vector stays as -1, -1
				if (!endU && !boundedInMap({ start.x, start.y - i }))
				{
					out.a = { start.x, start.y - (i - 1) };
					endU = true;
				}
				// Runs into a piece, vector becomes that position
				else if (!endU && isPieceAt({ start.x, start.y - i }))
				{
					out.a = { start.x, start.y - i };
					endU = true;
				}

				// Never runs into a piece, vector stays as -1, -1
				if (!endD && !boundedInMap({ start.x, start.y + i }))
				{
					out.b = { start.x, start.y + (i - 1) };
					endD = true;
				}
				// Runs into a piece, vector becomes that position
				else if (!endD && isPieceAt({ start.x, start.y + i }))
				{
					out.b = { start.x, start.y + i };
					endD = true;
				}
			}

			return out;
		}

		// a = Top, b = Bottom
		Game::vi2dPair findOnPositiveDiagonal(olc::vi2d start)
		{
			bool endT = false, endB = false;
			Game::vi2dPair out;
			//out.a = { 0, 0 };
			//out.b = { 0, 0 };
			out.a = { -1,-1 };
			out.b = { -1,-1 };

			for (int i = 1; !endT || !endB; i++)
			{
				// Runs into a piece or wall, vector becomes that position
				if (!endT && !boundedInMap({ start.x + i, start.y - i }))
				{
					out.a = { start.x + (i - 1), start.y - (i - 1) };
					endT = true;
				}
				else if (!endT && isPieceAt({ start.x + i, start.y - i }))
				{
					out.a = { start.x + i, start.y - i };
					endT = true;
				}

				if (!endB && !boundedInMap({ start.x - i, start.y + i }))
				{
					out.b = { start.x - (i - 1), start.y + (i - 1) };
					endB = true;
				}
				else if (!endB && isPieceAt({ start.x - i, start.y + i }))
				{
					out.b = { start.x - i, start.y + i };
					endB = true;
				}
			}

			return out;
		}

		// a = Top, b = Bottom
		Game::vi2dPair findOnNegitiveDiagonal(olc::vi2d start)
		{
			bool endT = false, endB = false;
			Game::vi2dPair out;
			out.a = { -1,-1 };
			out.b = { -1,-1 };

			for (int i = 1; !endT || !endB; i++)
			{
				// Runs into a piece, vector becomes that position
				if (!endT && !boundedInMap({ start.x - i, start.y - i }))
				{
					out.a = { start.x - (i - 1), start.y - (i - 1) };
					endT = true;
				}
				else if (!endT && isPieceAt({ start.x - i, start.y - i }))
				{
					out.a = { start.x - i, start.y - i };
					endT = true;
				}

				if (!endB && !boundedInMap({ start.x + i, start.y + i }))
				{
					out.b = { start.x + (i - 1), start.y + (i - 1) };
					endB = true;
				}
				else if (!endB && isPieceAt({ start.x + i, start.y + i }))
				{
					out.b = { start.x + i, start.y + i };
					endB = true;
				}
			}

			return out;
		}

		bool isHorizontal(olc::vi2d pos1, olc::vi2d pos2) { return pos1.y == pos2.y && pos1.x != pos2.x; }
		bool isHorizontalBounded(olc::vi2d origin, olc::vi2d limit, olc::vi2d check)
		{ return isHorizontal(origin, limit) && isHorizontal(origin, check) && bounded(min(origin.x, limit.x), max(origin.x, limit.x), check.x); }
		bool isVertical(olc::vi2d pos1, olc::vi2d pos2) { return pos1.x == pos2.x && pos1.y != pos2.y; }
		bool isVerticalBounded(olc::vi2d origin, olc::vi2d limit, olc::vi2d check)
		{ return isVertical(origin, limit) && isVertical(origin, check) && bounded(min(origin.y, limit.y), max(origin.y, limit.y), check.y); }
		bool isStraight(olc::vi2d pos1, olc::vi2d pos2) { return isHorizontal(pos1, pos2) || isVertical(pos1, pos2); }
		bool isStraightBounded(olc::vi2d origin, olc::vi2d limit, olc::vi2d check)
		{ return isHorizontalBounded(origin, limit, check) || isVerticalBounded(origin, limit, check); }

		bool isPositiveDiagonal(olc::vi2d pos1, olc::vi2d pos2) { return (pos1 - pos2).x == -(pos1 - pos2).y; }
		bool isPositiveDiagonalBounded(olc::vi2d origin, olc::vi2d limit, olc::vi2d check)
		{ return isPositiveDiagonal(origin, limit) && isPositiveDiagonal(origin, check) && bounded(min(origin, limit), max(origin, limit), check); }
		bool isNegitiveDiagonal(olc::vi2d pos1, olc::vi2d pos2) { return (pos1 - pos2).x ==  (pos1 - pos2).y; }
		bool isNegitiveDiagonalBounded(olc::vi2d origin, olc::vi2d limit, olc::vi2d check)
		{ return isNegitiveDiagonal(origin, limit) && isNegitiveDiagonal(origin, check) && bounded(min(origin, limit), max(origin, limit), check); }
		bool isDiagonal(olc::vi2d pos1, olc::vi2d pos2) { return isPositiveDiagonal(pos1, pos2) || isNegitiveDiagonal(pos1, pos2); }
		bool isDiagonalBounded(olc::vi2d origin, olc::vi2d limit, olc::vi2d check)
		{ return isPositiveDiagonalBounded(origin, limit, check) || isNegitiveDiagonalBounded(origin, limit, check); }

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

		Debug::DebugPieceLogic |= Debug::Rook;

		board.updateAllLogic();

		return true;

		// Tests
		Log(std::to_string(board.isHorizontal({ 0, 2 }, { 32, 2 }) == true));
		Log(std::to_string(board.isHorizontal({ 3, 2 }, { -32, 2 }) == true));
		Log(std::to_string(board.isHorizontal({ 3, 12 }, { 32, 2 }) == false));
		Log(std::to_string(board.isHorizontal({ 3, -12 }, { 32, 2 }) == false));

		Log(std::to_string(board.isVertical({ 32, 5 }, { 32, 2 }) == true));
		Log(std::to_string(board.isVertical({ 32, 5 }, { 32, -2 }) == true));
		Log(std::to_string(board.isVertical({ 3, 5 }, { 32, 2 }) == false));
		Log(std::to_string(board.isVertical({ 3, 5 }, { -32, 2 }) == false));

		Log(std::to_string(board.isPositiveDiagonal({ 0, 0 }, { 5, 5 }) == true));
		Log(std::to_string(board.isPositiveDiagonal({ 22, 33 }, { 33, 44 }) == true));
		Log(std::to_string(board.isPositiveDiagonal({ 0, 0 }, { -5, -5 }) == true));
		Log(std::to_string(board.isPositiveDiagonal({ -22, -33 }, { -33, -44 }) == true));
		Log(std::to_string(board.isPositiveDiagonal({ 21, 32 }, { 64, 23 }) == false));
		Log(std::to_string(board.isPositiveDiagonal({ 21, -32 }, { -64, 23 }) == false));
		Log(std::to_string(board.isPositiveDiagonal({ 21, 32 }, { -64, -23 }) == false));
		Log(std::to_string(board.isPositiveDiagonal({ -22, 33 }, { -33, 44 }) == false));
		Log(std::to_string(board.isPositiveDiagonal({ 22, -33 }, { 33, -44 }) == false));

		Log(std::to_string(board.isNegitiveDiagonal({ 22, -33 }, { 33, -44 }) == true));
		Log(std::to_string(board.isNegitiveDiagonal({ -22, 33 }, { -33, 44 }) == true));
		Log(std::to_string(board.isNegitiveDiagonal({ 12, -33 }, { 23, -43 }) == false));
		Log(std::to_string(board.isNegitiveDiagonal({ 21, 32 }, { 64, 23 }) == false));
		Log(std::to_string(board.isNegitiveDiagonal({ 22, 33 }, { 33, 44 }) == false));
		Log(std::to_string(board.isNegitiveDiagonal({ 0, 0 }, { -5, -5 }) == false));
		Log(std::to_string(board.isNegitiveDiagonal({ -22, -33 }, { -33, -44 }) == false));

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