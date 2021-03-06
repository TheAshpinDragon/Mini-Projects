#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

#define TAB_SIZE 3
#define PARAGRAPH '\n'
#define END_LINE char(0)
#define NULL char(255)

// Override base class with your custom functionality
class TextEditor : public olc::PixelGameEngine
{
public:
	TextEditor()
	{
		// Name your application
		sAppName = "Macrosoft Letters";
	}

	static void Log(std::string msg)
	{
		std::cout << msg << std::endl;
	}

	static void Log(char msg)
	{
		std::cout << msg << std::endl;
	}

	static void Log(int msg)
	{
		std::cout << msg << std::endl;
	}

	struct Character {
		char rawChar;

		olc::Pixel color;
		float size;
		bool underscore, bold, italics;

		Character() : rawChar(NULL) {}

		Character(char raw) : rawChar(raw) {}

		void DrawChar(TextEditor* editor, olc::vi2d screenPos)
		{
			if (rawChar != 255)
			{
				std::string s;
				s += rawChar;
				editor->DrawStringDecal(screenPos, s);
			}
		}
	};

	enum eTextAlignment {
		LEFT,
		RIGHT,
		CENTER,
		JUSTIFIED
	};

	struct Line {
		std::vector<Character*> chars;
		olc::vi2d relitivePos;
		int maxCharHeight;

		Line() : maxCharHeight(8) {  }

		void AppendChar(Character* ch)
		{
			chars.push_back(ch);
		}

		void InsertChar(Character* ch, int ind)
		{
			chars.insert(chars.begin() + ind, ch);
		}

		void DeleteChar(int ind)
		{
			chars.erase(chars.begin() + ind);
		}

		void EndLine()
		{
			chars[chars.size() - 1]->rawChar = '\n';
		}

		void DrawLine(TextEditor* editor, int& lineYPos)
		{
			olc::vi2d charPos{0, lineYPos};
			for (Character* c : chars)
			{
				c->DrawChar(editor, charPos);
				charPos.x += 8;
				
				if (charPos.x >= 256)
					charPos = {0, lineYPos += 8};
			}
		}
	};

	struct Paragraph {
		std::vector<Line*> lines{};
		eTextAlignment alignment{eTextAlignment::LEFT};
		olc::vi2d rootPos{0, 0};
		int indentLevel{0}, spacing{0};
		bool singleIndent{false}, spaceBef{false}, spaceAft{false};

		Paragraph() { lines.push_back(new Line()); }
		Paragraph(olc::vi2d pos) : rootPos(pos) { lines.push_back(new Line()); }

		void DrawParagraph(TextEditor* editor, int& lineYPos)
		{
			for (Line* l : lines)
			{
				l->DrawLine(editor, lineYPos);
				lineYPos += 8;
			}
		}

		void DeleteLine(int ind)
		{
			lines.erase(lines.begin() + ind);
		}

		void End()
		{
			lines[lines.size() - 1]->chars.push_back(new Character('\n'));
		}

		void RemoveEnd()
		{
			lines[lines.size() - 1]->DeleteChar(lines[lines.size() - 1]->chars.size() - 1);
		}

		int Size()
		{
			return lines.size();
		}
	};

	struct CharPos {
		int PAGE, PARA, LINE, CHAR;

		CharPos(int page, int paragraph, int line, int character) : PAGE{ page }, PARA{ paragraph }, LINE{ line }, CHAR { character } {}
	};

	struct Page {
		std::vector<Paragraph*> paragraphs;
		// margines
		// Page color
		// Header/footer

		Page() { paragraphs.push_back(new Paragraph()); }

		void DrawPage(TextEditor* editor)
		{
			int paragraphYPos = 0;
			for (Paragraph* para : paragraphs)
			{
				para->DrawParagraph(editor, paragraphYPos);
			}
		}
		
		void NewParagraph(Paragraph* para = nullptr)
		{
			if (para != nullptr)
				paragraphs.push_back(para);
			else
				paragraphs.push_back(new Paragraph());
		}

		void DeleteLine(int ind)
		{
			CharPos linePos = GetLinePos(ind);

			paragraphs[linePos.PARA]->DeleteLine(linePos.LINE);

			if (paragraphs[linePos.PARA]->Size() == 0)
				paragraphs.erase(paragraphs.begin() + linePos.PARA);

			if (linePos.PARA - 1 == paragraphs.size() - 1)
				paragraphs[linePos.PARA - 1]->RemoveEnd();
		}

		CharPos GetLinePos(int ind)
		{
			int paragraphInd = 0;

			while (ind > paragraphs[paragraphInd]->Size() - 1)
			{
				ind -= paragraphs[paragraphInd]->Size();
				paragraphInd++;
			}

			return CharPos{-1, paragraphInd, ind, -1};
		}

		Line* GetLinePtr(int ind)
		{
			CharPos pos = GetLinePos(ind);
			return paragraphs[pos.PARA]->lines[pos.LINE];
		}

		Paragraph* GetParagraphPtr(int lineInd)
		{
			CharPos pos = GetLinePos(lineInd);
			return paragraphs[pos.PARA];
		}

		int GetLineNum()
		{
			int sum = 0;
			
			for (Paragraph* p : paragraphs)
				sum += p->Size();

			return sum;
		}
	};
	
	/*
	enum Key (0-95)
	{
		(0)
		NONE,

		(1-26) Alphabet
		A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,

		(27-36) Numeric
		K0()), K1(!), K2(@), K3(#), K4($), K5(%), K6(^), K7(&), K8(*), K9((),
		
		(37-48) Function
		F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
		
		(49-52) Navigation I
		UP, DOWN, LEFT, RIGHT,

		(53-54) Whitespace I
		SPACE, TAB,
		
		(55-56) Modifiers
		SHIFT, CTRL,
		
		(57-58) Editing I
		INS, DEL,
		
		(59-63) Navigation II
		HOME, END, PGUP, PGDN,
		
		(64-65) Editing II
		BACK, ESCAPE,
		
		(66-67) Whitespace II
		RETURN, ENTER,
		
		(68-69) Locks I
		PAUSE, SCROLL,
		
		(70-84) Numpad
		NP0(NP_INS), NP1(NP_END), NP2(NP_DOWN), NP3(NP_PGDN), NP4(NP_LEFT), NP5(N/A), NP6(NP_RIGHT), NP7(NP_HOME), NP8(NP_UP), NP9(NP_PGDN),
		NP_MUL, NP_DIV, NP_ADD, NP_SUB, NP_DECIMAL,
		
		(85-96) Special Characters
		PERIOD(.>), EQUALS(=+), COMMA(,<), MINUS(-_),
		OEM_1(;:), OEM_2(/?), OEM_3(`~), OEM_4([{), OEM_5(\|), OEM_6(]}), OEM_7('"), OEM_8(OTHER),

		(97) Locks II
		CAPS_LOCK,
		
		(98)
		ENUM_END
	};
	*/

	struct Document {

		struct CaretPos {
			Document* doc;
			int PAGE = 0, LINE = 0, CHAR = 0;

			CaretPos(Document* document) : doc(document) {}

			void DrawCaret(TextEditor* editor)
			{
				editor->FillRect({ (CHAR % 32) * 8, (LINE + CHAR / 32) * 8 }, {8, 8});
			}

			Page* GetPage() { return doc->pages[PAGE]; }
			Paragraph* GetParagraph() { return GetPage()->GetParagraphPtr(LINE); }
			Line* GetLine() { return GetPage()->GetLinePtr(LINE); }
			Character* GetChar() { return GetLine()->chars[CHAR]; }

			// Navigate pages
			bool PageUp() { if (PAGE > 0) { PAGE--; return true; } else return false; }
			bool TryPageUp() { if (PAGE > 0) return true; else return false; }
			bool PageDown() { if (doc->pages.size() - 1 > PAGE) { PAGE++; return true; } else return false; }
			bool PageWrapUp() { if (PageUp()) { LINE = GetPage()->GetLineNum() - 1; return true; } else return false; }
			bool TryPageWrapUp() { if (TryPageUp()) { return true; } else return false; }
			bool PageWrapDown() { if (PageDown()) { LINE = 0; return true; } else return false; }

			// Navigate lines
			bool LineUp() { if (LINE > 0) { LINE--; return true; } else return PageWrapUp(); }
			bool TryLineUp() { if (LINE > 0) { return true; } else return TryPageWrapUp(); }
			bool LineDown() { if (GetPage()->GetLineNum() - 1 > LINE) { LINE++; return true; } else return PageWrapDown(); }
			bool LineWrapUp() { if (LineUp()) { CHAR = GetLine()->chars.size(); return true; } else return false; }
			bool LineWrapDown() { if (LineDown()) { CHAR = 0; return true; } else return false; }
			bool LineEnd() { CHAR = GetLine()->chars.size(); }
			bool LineHome() { CHAR = 0; }

			// Navigate characters
			bool CharRight() { if (GetLine()->chars.size() > CHAR) { CHAR++; return true; } else return LineWrapDown(); }
			bool CharLeft() { if (CHAR > 0) { CHAR--; return true; } else return LineWrapUp(); }
			bool TabRight() { if (GetLine()->chars.size() > CHAR + TAB_SIZE) { CHAR += TAB_SIZE; return true; } else return LineWrapDown(); }
			bool TabLeft() { if (CHAR > TAB_SIZE) { CHAR -= TAB_SIZE; return true; } else return LineWrapUp(); }

			std::string to_string()
			{
				return "CHAR: " + std::to_string(CHAR) + " LINE: " + std::to_string(LINE) + " PARAGRAPH: " + std::to_string(GetPage()->GetLinePos(LINE).PARA) + " PAGE: " + std::to_string(PAGE);
			}

			// TODO: Navigate to next space (ctrl + right/left)
		};

		std::string rawContent;
		std::vector<Page*> pages;
		CaretPos caretPos = CaretPos(this); // , selectionStart;
		// bool textSelected;
		bool CAPSLOCK, NUMLOCK;
		
		std::vector<olc::Key> KeyList {
			olc::NONE,
			// Aphabet
			olc::A, olc::B, olc::C, olc::D, olc::E, olc::F, olc::G, olc::H, olc::I, olc::J, olc::K, olc::L, olc::M,
			olc::N, olc::O, olc::P, olc::Q, olc::R, olc::S, olc::T, olc::U, olc::V, olc::W, olc::X, olc::Y, olc::Z,
			// Top row (0-9 & special)
			olc::K0, olc::K1, olc::K2, olc::K3, olc::K4, olc::K5, olc::K6, olc::K7, olc::K8, olc::K9,
			// F-keys
			olc::F1, olc::F2, olc::F3, olc::F4, olc::F5, olc::F6, olc::F7, olc::F8, olc::F9, olc::F10, olc::F11, olc::F12,
			// Arrow keys
			olc::UP, olc::DOWN, olc::LEFT, olc::RIGHT,
			// Whitespace			// Modifiers
			olc::SPACE, olc::TAB,	olc::SHIFT, olc::CTRL,
			// Six-key island, mouseless navigation
			olc::INS, olc::DEL, olc::HOME, olc::END, olc::PGUP, olc::PGDN, olc::BACK, olc::ESCAPE,
			// Newline					// Misc
			olc::RETURN, olc::ENTER,	olc::PAUSE, olc::SCROLL,
			// Numpad
			olc::NP0, olc::NP1, olc::NP2, olc::NP3, olc::NP4, olc::NP5, olc::NP6, olc::NP7, olc::NP8, olc::NP9,
			olc::NP_MUL, olc::NP_DIV, olc::NP_ADD, olc::NP_SUB, olc::NP_DECIMAL,
			// PERIOD(.>), EQUALS(=+), COMMA(,<), MINUS(-_), OEM_1(;:), OEM_2(/?), OEM_3(`~), OEM_4([{), OEM_5(\|), OEM_6(]}), OEM_7('"), OEM_8(OTHER),
			olc::PERIOD, olc::EQUALS, olc::COMMA, olc::MINUS, olc::OEM_1, olc::OEM_2, olc::OEM_3, olc::OEM_4, olc::OEM_5, olc::OEM_6, olc::OEM_7, olc::OEM_8,
			olc::CAPS_LOCK,
			olc::ENUM_END
		};
		std::map<olc::Key, char> KeyCharMap {
			{olc::NONE, 0},
			{olc::A, 'a'}, {olc::B, 'b'}, {olc::C, 'c'}, {olc::D, 'd'}, {olc::E, 'e'}, {olc::F, 'f'}, {olc::G, 'g'}, {olc::H, 'h'}, {olc::I, 'i'}, {olc::J, 'j'}, {olc::K, 'k'}, {olc::L, 'l'}, {olc::M, 'm'},
			{olc::N, 'n'}, {olc::O, 'o'}, {olc::P, 'p'}, {olc::Q, 'q'}, {olc::R, 'r'}, {olc::S, 's'}, {olc::T, 't'}, {olc::U, 'u'}, {olc::V, 'v'}, {olc::W, 'w'}, {olc::X, 'x'}, {olc::Y, 'y'}, {olc::Z, 'z'},
			{olc::K0, '0'}, {olc::K1, '1'}, {olc::K2, '2'}, {olc::K3, '3'}, {olc::K4, '4'}, {olc::K5, '5'}, {olc::K6, '6'}, {olc::K7, '7'}, {olc::K8, '8'}, {olc::K9, '9'},
			{olc::F1, 0}, {olc::F2, 0}, {olc::F3, 0}, {olc::F4, 0}, {olc::F5, 0}, {olc::F6, 0}, {olc::F7, 0}, {olc::F8, 0}, {olc::F9, 0}, {olc::F10, 0}, {olc::F11, 0}, {olc::F12, 0},
			{olc::UP, 0}, {olc::DOWN, 0}, {olc::LEFT, 0}, {olc::RIGHT,0},
			{olc::SPACE, ' '}, {olc::TAB, '\t'}, {olc::SHIFT, 0}, {olc::CTRL, 0}, {olc::INS, 0}, {olc::DEL, 0}, {olc::HOME, 0}, {olc::END, 0}, {olc::PGUP, 0}, {olc::PGDN, 0},
			{olc::BACK, 0}, {olc::ESCAPE, 0}, {olc::RETURN, '/n'}, {olc::ENTER, '\n'}, {olc::PAUSE, 0}, {olc::SCROLL, 0},
			{olc::NP0, 0}, {olc::NP1, 0}, {olc::NP2, 0}, {olc::NP3, 0}, {olc::NP4, 0}, {olc::NP5, 0}, {olc::NP6, 0}, {olc::NP7, 0}, {olc::NP8, 0}, {olc::NP9, 0},
			{olc::NP_MUL, '*'}, {olc::NP_DIV, '/'}, {olc::NP_ADD, '+'}, {olc::NP_SUB, '-'}, {olc::NP_DECIMAL, '.'},
			{olc::PERIOD, '.'}, {olc::EQUALS, '='}, {olc::COMMA, ','}, {olc::MINUS, '-'},
			{olc::OEM_1, ';'}, {olc::OEM_2, '/'}, {olc::OEM_3, '`'}, {olc::OEM_4, '['}, {olc::OEM_5, '\\'}, {olc::OEM_6, ']'}, {olc::OEM_7, '\''}, {olc::OEM_8, 0},
			{olc::CAPS_LOCK, 0}, {olc::ENUM_END, 0}
		};
		std::map<olc::Key, char> ShiftKeyCharMap {
			{olc::NONE, 0},
			{olc::A, 'A'}, {olc::B, 'B'}, {olc::C, 'C'}, {olc::D, 'D'}, {olc::E, 'E'}, {olc::F, 'F'}, {olc::G, 'G'}, {olc::H, 'H'}, {olc::I, 'I'}, {olc::J, 'J'}, {olc::K, 'K'}, {olc::L, 'L'}, {olc::M, 'M'},
			{olc::N, 'N'}, {olc::O, 'O'}, {olc::P, 'P'}, {olc::Q, 'Q'}, {olc::R, 'R'}, {olc::S, 'S'}, {olc::T, 'T'}, {olc::U, 'U'}, {olc::V, 'V'}, {olc::W, 'W'}, {olc::X, 'X'}, {olc::Y, 'Y'}, {olc::Z, 'Z'},
			{olc::K0, ')'}, {olc::K1, '!'}, {olc::K2, '@'}, {olc::K3, '#'}, {olc::K4, '$'}, {olc::K5, '%'}, {olc::K6, '^'}, {olc::K7, '&'}, {olc::K8, '*'}, {olc::K9, '('},
			{olc::F1, 0}, {olc::F2, 0}, {olc::F3, 0}, {olc::F4, 0}, {olc::F5, 0}, {olc::F6, 0}, {olc::F7, 0}, {olc::F8, 0}, {olc::F9, 0}, {olc::F10, 0}, {olc::F11, 0}, {olc::F12, 0},
			{olc::UP, 0}, {olc::DOWN, 0}, {olc::LEFT, 0}, {olc::RIGHT, 0},
			{olc::SPACE, ' '}, {olc::TAB, '\t'}, {olc::SHIFT, 0}, {olc::CTRL, 0}, {olc::INS, 0}, {olc::DEL, 0}, {olc::HOME, 0}, {olc::END, 0}, {olc::PGUP, 0}, {olc::PGDN, 0},
			{olc::BACK, 0}, {olc::ESCAPE, 0}, {olc::RETURN, '\n'}, {olc::ENTER, '\n'}, {olc::PAUSE, 0}, {olc::SCROLL, 0},
			{olc::NP0, '0'}, {olc::NP1, '1'}, {olc::NP2, '2'}, {olc::NP3, '3'}, {olc::NP4, '4'}, {olc::NP5, '5'}, {olc::NP6, '6'}, {olc::NP7, '7'}, {olc::NP8, '8'}, {olc::NP9, '9'},
			{olc::NP_MUL, '*'}, {olc::NP_DIV, '/'}, {olc::NP_ADD, '+'}, {olc::NP_SUB, '-'}, {olc::NP_DECIMAL, 0},
			{olc::PERIOD, '>'}, {olc::EQUALS, '+'}, {olc::COMMA, '<'}, {olc::MINUS, '_'},
			{olc::OEM_1, ':'}, {olc::OEM_2, '?'}, {olc::OEM_3, '~'}, {olc::OEM_4, '{'}, {olc::OEM_5, '|'}, {olc::OEM_6, '}'}, {olc::OEM_7, '"'}, {olc::OEM_8, 0},
			{olc::CAPS_LOCK, 0}, {olc::ENUM_END, 0}
		};

		const std::vector<int> whitespace = { 53, 54, 66, 67 };

		Document()
		{
			pages.push_back(new Page());
		}

		void DrawDoc(TextEditor* editor)
		{
			//editor->DrawStringDecal({0,0}, rawContent);

			for (Page* p : pages)
				p->DrawPage(editor);

			caretPos.DrawCaret(editor);
		}

		void AddCharacter(char d)
		{
			if(d == '\n')
			{
				caretPos.GetParagraph()->End();
				caretPos.GetPage()->NewParagraph();
				rawContent += '\n';
				caretPos.LineWrapDown();

				Log(caretPos.to_string());
				return;
			}

			if (d == '\t' && caretPos.GetLine()->chars.size() + 3 < 32)
			{
				//rawContent += '\t';
				//caretPos.GetLine()->chars.push_back({new Character(' ')});
				//caretPos.TabRight();
				return;
			}

			if (caretPos.CHAR >= 32)
			{
				caretPos.LineWrapDown();
			}

			rawContent += d;
			caretPos.GetLine()->chars.push_back(new Character(d));
			caretPos.CharRight();

			Log("Draw char: " + std::string(1, d) + " " + caretPos.to_string());
		}

		void DeleteCharacter()
		{
			// Return if there is nothing to delete
			if (rawContent.size() == 0)
				return;

			// If the current line is empty, remove it
			if (caretPos.CHAR == 0 && caretPos.TryLineUp())
			{
				rawContent = rawContent.substr(0, rawContent.size() - 1);
				caretPos.GetPage()->DeleteLine(caretPos.LINE);
				caretPos.CharLeft();

				//Log(caretPos.to_string());
				return;
			}

			rawContent = rawContent.substr(0, rawContent.size() - 1);
			caretPos.GetLine()->chars.erase(caretPos.GetLine()->chars.begin() + caretPos.CHAR - 1);
			caretPos.CharLeft();

			Log(caretPos.to_string());
		}

		void PollKeyboard(TextEditor* editor)
		{
			bool SHIFT = editor->GetKey(olc::SHIFT).bHeld;
			bool CTRL = editor->GetKey(olc::CTRL).bHeld;
			if (editor->GetKey(olc::CAPS_LOCK).bPressed) CAPSLOCK = !CAPSLOCK;
			if (editor->GetKey(olc::NUM_LOCK).bPressed) NUMLOCK = !NUMLOCK;

			if (CTRL)
			{
				return;
			}
			
			if (editor->GetKey(olc::BACK).bPressed)
			{
				DeleteCharacter();
			}

			// Space, tab, return, enter(numpad)
			for (int white : whitespace)
				if (editor->GetKey(KeyList[white]).bPressed)
				{
					AddCharacter(KeyCharMap[KeyList[white]]);
					return;
				}

			// Alphabet
			for (int alpha = 1; alpha <= 26; alpha++)
				if (editor->GetKey(KeyList[alpha]).bPressed) // 
				{
					AddCharacter(!(SHIFT ^ CAPSLOCK) ? KeyCharMap[KeyList[alpha]] : ShiftKeyCharMap[KeyList[alpha]]);
					//PrintCh('D');
					return;
				}
			
			// Numbers
			for (int num = 27; num <= 36; num++)
				if (editor->GetKey(KeyList[num]).bPressed)
				{
					AddCharacter(!SHIFT ? KeyCharMap[KeyList[num]] : ShiftKeyCharMap[KeyList[num]]);
					return;
				}

			// Special
			for (int special = 84; special <= 95; special++)
				if (editor->GetKey(KeyList[special]).bPressed)
				{
					AddCharacter(!SHIFT ? KeyCharMap[KeyList[special]] : ShiftKeyCharMap[KeyList[special]]);
					return;
				}
		}
	};

public:
	Document doc;

	bool OnUserCreate() override
	{
		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		Clear(olc::BLACK);
		doc.PollKeyboard(this);
		doc.DrawDoc(this);
		return true;
	}
};

int main()
{
	TextEditor demo;
	if (demo.Construct(256, 240, 4, 4))
		demo.Start();
	return 0;
}