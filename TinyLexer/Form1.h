#pragma once
#include <string>
#include <vector>
#include <msclr/marshal_cppstd.h>
using namespace std;

namespace TinyLexer {

	using namespace System;
	using namespace System::Windows::Forms;
	using namespace System::Drawing;
	using namespace System::Collections::Generic;

	// ================================================================
	//  TOKEN TYPES
	// ================================================================
	enum class TokenType {
		IF_KW, THEN_KW, ELSE_KW, END_KW,
		REPEAT_KW, UNTIL_KW, READ_KW, WRITE_KW,
		ID, NUMBER, STRING_LIT,
		ADDOP, SUBOP, MULOP, DIVOP,
		COMPARISONOP, ASSIGNMENTOP,
		SEMICOLON, COMMA, PUNCTUATION,
		ENDOFFILE, LEXERROR
	};

	// ================================================================
	//  TOKEN STRUCT
	// ================================================================
	struct Token {
		TokenType   type;
		string      lexeme;
		int         line;
		int         col;
	};

	// ================================================================
	//  TOKEN TYPE TO STRING
	// ================================================================
	String^ tokenName(TokenType t) {
		switch (t) {
		case TokenType::IF_KW:        return "IF_KW";
		case TokenType::THEN_KW:      return "THEN_KW";
		case TokenType::ELSE_KW:      return "ELSE_KW";
		case TokenType::END_KW:       return "END_KW";
		case TokenType::REPEAT_KW:    return "REPEAT_KW";
		case TokenType::UNTIL_KW:     return "UNTIL_KW";
		case TokenType::READ_KW:      return "READ_KW";
		case TokenType::WRITE_KW:     return "WRITE_KW";
		case TokenType::ID:           return "ID";
		case TokenType::NUMBER:       return "NUMBER";
		case TokenType::STRING_LIT:   return "STRING";
		case TokenType::ADDOP:        return "ADDOP";
		case TokenType::SUBOP:        return "SUBOP";
		case TokenType::MULOP:        return "MULOP";
		case TokenType::DIVOP:        return "DIVOP";
		case TokenType::COMPARISONOP: return "COMPARISONOP";
		case TokenType::ASSIGNMENTOP: return "ASSIGNMENTOP";
		case TokenType::SEMICOLON:    return "SEMICOLON";
		case TokenType::COMMA:        return "COMMA";
		case TokenType::PUNCTUATION:  return "PUNCTUATION";
		case TokenType::ENDOFFILE:    return "EOF";
		default:                      return "ERROR";
		}
	}

	// ================================================================
	//  LEXER CLASS (pure C++ standard — no CLR types inside)
	// ================================================================
	class Lexer {
		string src;
		size_t pos = 0;
		int    line = 1, col = 1;

		char peek(int offset = 0) {
			size_t i = pos + offset;
			return (i < src.size()) ? src[i] : '\0';
		}
		char advance() {
			char c = src[pos++];
			if (c == '\n') { line++; col = 1; }
			else { col++; }
			return c;
		}
		bool match(char expected) {
			if (pos < src.size() && src[pos] == expected) { advance(); return true; }
			return false;
		}
		void skipWhitespace() {
			while (pos < src.size() && isspace((unsigned char)peek()))
				advance();
		}
		void skipComment() {
			while (pos < src.size()) {
				if (peek() == '}') { advance(); return; }
				advance();
			}
		}

	public:
		explicit Lexer(const string& source) : src(source) {}

		Token nextToken() {
			skipWhitespace();
			if (pos >= src.size()) return { TokenType::ENDOFFILE, "EOF", line, col };

			int sl = line, sc = col;
			char c = advance();

			// Comment
			if (c == '{') { skipComment(); return nextToken(); }

			// String literal
			if (c == '"') {
				string lex = "\"";
				while (pos < src.size() && peek() != '"' && peek() != '\n')
					lex += advance();
				if (pos >= src.size() || peek() == '\n')
					return { TokenType::LEXERROR, "unterminated string", sl, sc };
				lex += advance();
				return { TokenType::STRING_LIT, lex, sl, sc };
			}

			// Number
			if (isdigit((unsigned char)c)) {
				string lex(1, c);
				while (pos < src.size() && isdigit((unsigned char)peek()))
					lex += advance();
				return { TokenType::NUMBER, lex, sl, sc };
			}

			// Identifier / Reserved word
			if (isalpha((unsigned char)c)) {
				string lex(1, c);
				while (pos < src.size() &&
					(isalpha((unsigned char)peek()) || isdigit((unsigned char)peek())))
					lex += advance();
				if (lex == "if")     return { TokenType::IF_KW,     lex, sl, sc };
				if (lex == "then")   return { TokenType::THEN_KW,   lex, sl, sc };
				if (lex == "else")   return { TokenType::ELSE_KW,   lex, sl, sc };
				if (lex == "end")    return { TokenType::END_KW,    lex, sl, sc };
				if (lex == "repeat") return { TokenType::REPEAT_KW, lex, sl, sc };
				if (lex == "until")  return { TokenType::UNTIL_KW,  lex, sl, sc };
				if (lex == "read")   return { TokenType::READ_KW,   lex, sl, sc };
				if (lex == "write")  return { TokenType::WRITE_KW,  lex, sl, sc };
				return { TokenType::ID, lex, sl, sc };
			}

			// Operators and symbols
			switch (c) {
			case '+': return { TokenType::ADDOP,        "+",  sl, sc };
			case '-': return { TokenType::SUBOP,        "-",  sl, sc };
			case '*': return { TokenType::MULOP,        "*",  sl, sc };
			case '/': return { TokenType::DIVOP,        "/",  sl, sc };
			case '=': return { TokenType::COMPARISONOP, "=",  sl, sc };
			case '<': return { TokenType::COMPARISONOP, "<",  sl, sc };
			case ';': return { TokenType::SEMICOLON,    ";",  sl, sc };
			case ',': return { TokenType::COMMA,        ",",  sl, sc };
			case '(': return { TokenType::PUNCTUATION,  "(",  sl, sc };
			case ')': return { TokenType::PUNCTUATION,  ")",  sl, sc };
			case ':':
				if (match('=')) return { TokenType::ASSIGNMENTOP, ":=", sl, sc };
				return { TokenType::LEXERROR, ":", sl, sc };
			}
			return { TokenType::LEXERROR, string(1, c), sl, sc };
		}

		vector<Token> tokenize() {
			vector<Token> tokens;
			while (true) {
				Token tok = nextToken();
				tokens.push_back(tok);
				if (tok.type == TokenType::ENDOFFILE) break;
			}
			return tokens;
		}
	};

	// ================================================================
	//  WINDOWS FORM
	// ================================================================
	public ref class Form1 : public Form {

		// ── Controls ────────────────────────────────────────────────
		RichTextBox^ txtSource;
		DataGridView^ dgvTokens;
		Button^ btnScan;
		Button^ btnClear;
		Label^ lblInput;
		Label^ lblOutput;
		Label^ lblStatus;
		Panel^ pnlHeader;
		Label^ lblTitle;

	public:
		Form1() { InitializeComponent(); }

	protected:
		~Form1() {}

	private:

		void addCol(String^ header, String^ name, int fillWeight, DataGridViewContentAlignment align) {
			DataGridViewTextBoxColumn^ col = gcnew DataGridViewTextBoxColumn();
			col->HeaderText = header;
			col->Name = name;
			col->FillWeight = (float)fillWeight;
			col->DefaultCellStyle->Alignment = align;
			col->SortMode = DataGridViewColumnSortMode::NotSortable;
			dgvTokens->Columns->Add(col);
		}

		// ── UI Builder ───────────────────────────────────────────────
		void InitializeComponent() {

			// ── Form properties ─────────────────────────────────────
			this->Text = L"Tiny Language _ Lexical Analyzer";
			this->Size = Drawing::Size(1100, 750);
			this->MinimumSize = Drawing::Size(900, 600);
			this->StartPosition = FormStartPosition::CenterScreen;
			this->BackColor = Color::FromArgb(240, 244, 248);
			this->Font = gcnew Drawing::Font("Segoe UI", 9.5f);

			// ── Header panel ─────────────────────────────────────────
			pnlHeader = gcnew Panel();
			pnlHeader->Dock = DockStyle::Top;
			pnlHeader->Height = 60;
			pnlHeader->BackColor = Color::FromArgb(30, 60, 114);

			lblTitle = gcnew Label();
			lblTitle->Text = L"TINY Language Lexical Analyzer (Scanner)";
			lblTitle->ForeColor = Color::White;
			lblTitle->Font = gcnew Drawing::Font("Segoe UI", 14.0f, FontStyle::Bold);
			lblTitle->Dock = DockStyle::Fill;
			lblTitle->TextAlign = ContentAlignment::MiddleLeft;
			pnlHeader->Controls->Add(lblTitle);

			// ── Input label ──────────────────────────────────────────
			lblInput = gcnew Label();
			lblInput->Text = L"Source Code Input:";
			lblInput->Font = gcnew Drawing::Font("Segoe UI", 10.0f, FontStyle::Bold);
			lblInput->ForeColor = Color::FromArgb(30, 60, 114);
			lblInput->Location = Point(10, 70);
			lblInput->Size = Drawing::Size(300, 22);

			// ── Source code text area ────────────────────────────────
			txtSource = gcnew RichTextBox();
			txtSource->Location = Point(10, 95);
			txtSource->Size = Drawing::Size(1065, 200);
			txtSource->Font = gcnew Drawing::Font("Consolas", 10.5f);
			txtSource->BackColor = Color::FromArgb(30, 30, 46);
			txtSource->ForeColor = Color::FromArgb(202, 211, 245);
			txtSource->BorderStyle = BorderStyle::FixedSingle;
			txtSource->ScrollBars = RichTextBoxScrollBars::Both;
			txtSource->WordWrap = false;
			txtSource->Text =
				L"{ Sample Tiny program — computes factorial }\r\n"
				L"read x;\r\n"
				L"if 0 < x then\r\n"
				L"    fact := 1;\r\n"
				L"    repeat\r\n"
				L"        fact := fact * x;\r\n"
				L"        x := x - 1\r\n"
				L"    until x = 0;\r\n"
				L"    write fact\r\n"
				L"end";

			// ── Scan Button ──────────────────────────────────────────
			btnScan = gcnew Button();
			btnScan->Text = L"Scan / Tokenize";
			btnScan->Location = Point(10, 308);
			btnScan->Size = Drawing::Size(180, 40);
			btnScan->BackColor = Color::FromArgb(30, 60, 114);
			btnScan->ForeColor = Color::White;
			btnScan->FlatStyle = FlatStyle::Flat;
			btnScan->FlatAppearance->BorderSize = 0;
			btnScan->Font = gcnew Drawing::Font("Segoe UI", 10.5f, FontStyle::Bold);
			btnScan->Cursor = Cursors::Hand;
			btnScan->Click += gcnew EventHandler(this, &Form1::btnScan_Click);

			// ── Clear Button ─────────────────────────────────────────
			btnClear = gcnew Button();
			btnClear->Text = L"Clear";
			btnClear->Location = Point(200, 308);
			btnClear->Size = Drawing::Size(120, 40);
			btnClear->BackColor = Color::FromArgb(180, 70, 70);
			btnClear->ForeColor = Color::White;
			btnClear->FlatStyle = FlatStyle::Flat;
			btnClear->FlatAppearance->BorderSize = 0;
			btnClear->Font = gcnew Drawing::Font("Segoe UI", 10.5f, FontStyle::Bold);
			btnClear->Cursor = Cursors::Hand;
			btnClear->Click += gcnew EventHandler(this, &Form1::btnClear_Click);

			// ── Output label ─────────────────────────────────────────
			lblOutput = gcnew Label();
			lblOutput->Text = L"Token Output Table:";
			lblOutput->Font = gcnew Drawing::Font("Segoe UI", 10.0f, FontStyle::Bold);
			lblOutput->ForeColor = Color::FromArgb(30, 60, 114);
			lblOutput->Location = Point(10, 358);
			lblOutput->Size = Drawing::Size(300, 22);

			// ── DataGridView ─────────────────────────────────────────
			dgvTokens = gcnew DataGridView();
			dgvTokens->Location = Point(10, 382);
			dgvTokens->Size = Drawing::Size(1065, 300);
			dgvTokens->ReadOnly = true;
			dgvTokens->AllowUserToAddRows = false;
			dgvTokens->AllowUserToDeleteRows = false;
			dgvTokens->AllowUserToResizeRows = false;
			dgvTokens->RowHeadersVisible = false;
			dgvTokens->SelectionMode = DataGridViewSelectionMode::FullRowSelect;
			dgvTokens->AutoSizeColumnsMode = DataGridViewAutoSizeColumnsMode::Fill;
			dgvTokens->BackgroundColor = Color::White;
			dgvTokens->BorderStyle = BorderStyle::FixedSingle;
			dgvTokens->GridColor = Color::FromArgb(210, 215, 225);
			dgvTokens->Font = gcnew Drawing::Font("Consolas", 10.0f);
			dgvTokens->ColumnHeadersDefaultCellStyle->Font =
				gcnew Drawing::Font("Segoe UI", 10.0f, FontStyle::Bold);
			dgvTokens->ColumnHeadersDefaultCellStyle->BackColor =
				Color::FromArgb(30, 60, 114);
			dgvTokens->ColumnHeadersDefaultCellStyle->ForeColor = Color::White;
			dgvTokens->ColumnHeadersDefaultCellStyle->Alignment =
				DataGridViewContentAlignment::MiddleCenter;
			dgvTokens->ColumnHeadersHeight = 36;
			dgvTokens->EnableHeadersVisualStyles = false;
			dgvTokens->AlternatingRowsDefaultCellStyle->BackColor =
				Color::FromArgb(245, 248, 255);
			dgvTokens->CellFormatting +=
				gcnew DataGridViewCellFormattingEventHandler(
					this, &Form1::dgv_CellFormatting);

			addCol(L"#", L"colIdx", 8, DataGridViewContentAlignment::MiddleCenter);
			addCol(L"Token Type", L"colType", 25, DataGridViewContentAlignment::MiddleCenter);
			addCol(L"Lexeme", L"colLexeme", 30, DataGridViewContentAlignment::MiddleLeft);
			addCol(L"Line", L"colLine", 8, DataGridViewContentAlignment::MiddleCenter);
			addCol(L"Column", L"colCol", 8, DataGridViewContentAlignment::MiddleCenter);

			// ── Status label ─────────────────────────────────────────
			lblStatus = gcnew Label();
			lblStatus->Text = L"Ready. Enter Tiny source code above and click Scan.";
			lblStatus->Location = Point(10, 690);
			lblStatus->Size = Drawing::Size(1065, 22);
			lblStatus->ForeColor = Color::FromArgb(80, 100, 130);
			lblStatus->Font = gcnew Drawing::Font("Segoe UI", 9.0f, FontStyle::Italic);

			// ── Add all controls ─────────────────────────────────────
			this->Controls->Add(pnlHeader);
			this->Controls->Add(lblInput);
			this->Controls->Add(txtSource);
			this->Controls->Add(btnScan);
			this->Controls->Add(btnClear);
			this->Controls->Add(lblOutput);
			this->Controls->Add(dgvTokens);
			this->Controls->Add(lblStatus);

			// ── Resize handler ───────────────────────────────────────
			this->Resize += gcnew EventHandler(this, &Form1::Form1_Resize);
		}

		// ── Resize: keep controls stretched ─────────────────────────
		void Form1_Resize(Object^ sender, EventArgs^ e) {
			int w = this->ClientSize.Width - 20;
			txtSource->Width = w;
			dgvTokens->Width = w;
			lblStatus->Width = w;
		}

		// ── SCAN button handler ──────────────────────────────────────
		void btnScan_Click(Object^ sender, EventArgs^ e) {
			dgvTokens->Rows->Clear();

			String^ managed = txtSource->Text;
			if (managed->Trim()->Length == 0) {
				lblStatus->Text = L"Input is empty. Please enter some Tiny source code.";
				lblStatus->ForeColor = Color::OrangeRed;
				return;
			}

			// Convert managed String^ → std::string
			string src = msclr::interop::marshal_as<string>(managed);

			// Run the lexer
			Lexer lexer(src);
			vector<Token> tokens = lexer.tokenize();

			int idx = 1, errCount = 0;
			for (const Token& tok : tokens) {
				if (tok.type == TokenType::ENDOFFILE) break;

				String^ typeStr = tokenName(tok.type);
				String^ lexStr = gcnew String(tok.lexeme.c_str());
				String^ lineStr = tok.line.ToString();
				String^ colStr = tok.col.ToString();
				String^ idxStr = idx.ToString();

				dgvTokens->Rows->Add(idxStr, typeStr, lexStr, lineStr, colStr);
				if (tok.type == TokenType::LEXERROR) errCount++;
				idx++;
			}

			int total = idx - 1;
			if (errCount > 0) {
				lblStatus->Text = String::Format(
					L"Scan complete — {0} token(s) found, {1} error(s) detected. (Errors shown in red)",
					total, errCount);
				lblStatus->ForeColor = Color::OrangeRed;
			}
			else {
				lblStatus->Text = String::Format(
					L"Scan complete — {0} token(s) found successfully. No errors detected.",
					total);
				lblStatus->ForeColor = Color::FromArgb(0, 130, 80);
			}
		}

		// ── CLEAR button handler ─────────────────────────────────────
		void btnClear_Click(Object^ sender, EventArgs^ e) {
			txtSource->Clear();
			dgvTokens->Rows->Clear();
			lblStatus->Text = L"Cleared. Enter new Tiny source code and click Scan.";
			lblStatus->ForeColor = Color::FromArgb(80, 100, 130);
		}

		// ── Color rows by token category ────────────────────────────
		void dgv_CellFormatting(Object^ sender, DataGridViewCellFormattingEventArgs^ e) {
			if (e->RowIndex < 0) return;
			DataGridViewRow^ row = dgvTokens->Rows[e->RowIndex];
			String^ type = safe_cast<String^>(row->Cells["colType"]->Value);
			if (type == nullptr) return;

			// Keywords → blue tint
			if (type->EndsWith("_KW")) {
				row->DefaultCellStyle->BackColor = Color::FromArgb(220, 235, 255);
				row->DefaultCellStyle->ForeColor = Color::FromArgb(10, 50, 150);
				return;
			}
			// Identifiers → green tint
			if (type == "ID") {
				row->DefaultCellStyle->BackColor = Color::FromArgb(220, 255, 230);
				row->DefaultCellStyle->ForeColor = Color::FromArgb(0, 100, 40);
				return;
			}
			// Numbers → orange tint
			if (type == "NUMBER") {
				row->DefaultCellStyle->BackColor = Color::FromArgb(255, 245, 215);
				row->DefaultCellStyle->ForeColor = Color::FromArgb(150, 80, 0);
				return;
			}
			// Strings → purple tint
			if (type == "STRING") {
				row->DefaultCellStyle->BackColor = Color::FromArgb(245, 220, 255);
				row->DefaultCellStyle->ForeColor = Color::FromArgb(100, 0, 150);
				return;
			}
			// Operators → yellow tint
			if (type == "ADDOP" || type == "SUBOP" || type == "MULOP" || type == "DIVOP" ||
				type == "COMPARISONOP" || type == "ASSIGNMENTOP") {
				row->DefaultCellStyle->BackColor = Color::FromArgb(255, 255, 210);
				row->DefaultCellStyle->ForeColor = Color::FromArgb(120, 100, 0);
				return;
			}
			// Errors → red
			if (type == "ERROR") {
				row->DefaultCellStyle->BackColor = Color::FromArgb(255, 210, 210);
				row->DefaultCellStyle->ForeColor = Color::FromArgb(180, 0, 0);
				return;
			}
		}
	};
}
