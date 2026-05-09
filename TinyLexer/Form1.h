#pragma once
#include <string>
#include <vector>
#include <sstream>
#include <stdexcept>
#include <utility>      // Required for std::pair
#include <cctype>       // Required for character checks (isspace, isdigit, isalpha)
#include <msclr/marshal_cppstd.h> // Required for System::String to std::string conversion

using namespace std;

namespace TinyLexer {

	using namespace System;
	using namespace System::Windows::Forms;
	using namespace System::Drawing;
	using namespace System::Collections::Generic;

	// ================================================================
	//  TOKEN TYPES (Phase 1)
	//  Defines all valid token categories in the Tiny Language.
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

	// Data structure to hold token information
	struct Token {
		TokenType   type;
		string      lexeme;
		int         line;
		int         col;
	};

	// Helper function to convert TokenType enum to a readable String
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
		case TokenType::LEXERROR:     return "LEXERROR";
		default:                      return "UNKNOWN";
		}
	}

	// ================================================================
	//  LEXER CLASS (Phase 1)
	//  Scans the raw source code string and converts it into a 
	//  sequence of recognized Tokens.
	// ================================================================
	class Lexer {
		string src;
		size_t pos = 0;
		int    line = 1;
		int    col = 0;

		// Lookahead character without consuming it
		char peek(int offset = 0) const {
			size_t i = pos + offset;
			return (i < src.size()) ? src[i] : '\0';
		}

		// Consume the next character and update line/col trackers
		char advance() {
			char c = src[pos++];
			if (c == '\n') { line++; col = 0; }
			else { col++; }
			return c;
		}

		// Match the current character with an expected one
		bool match(char expected) {
			if (pos < src.size() && src[pos] == expected) { advance(); return true; }
			return false;
		}

		// Ignore spaces, tabs, and newlines
		void skipWhitespace() {
			while (pos < src.size() && isspace((unsigned char)peek()))
				advance();
		}

		// Ignore text inside curly braces { ... }
		bool skipComment() {
			while (pos < src.size()) {
				if (peek() == '}') { advance(); return true; }
				advance();
			}
			return false; // Reached EOF without closing brace
		}

	public:
		explicit Lexer(const string& source) : src(source) {}

		// Extracts the next valid token from the source code
		Token nextToken() {
			skipWhitespace();
			if (pos >= src.size())
				return { TokenType::ENDOFFILE, "EOF", line, col };

			int sl = line;
			int sc = col + 1;
			char c = advance();

			// Handle Comments
			if (c == '{') {
				if (!skipComment())
					return { TokenType::LEXERROR, "{", sl, sc }; // Unterminated comment
				return nextToken();
			}

			// Handle Strings
			if (c == '"') {
				string lex = "\"";
				while (pos < src.size()) {
					if (peek() == '\n')
						return { TokenType::LEXERROR, "unterminated string", sl, sc };
					if (peek() == '"') {
						advance();
						lex += '"';
						return { TokenType::STRING_LIT, lex, sl, sc };
					}
					lex += advance();
				}
				return { TokenType::LEXERROR, "unterminated string", sl, sc };
			}

			// Handle Numbers
			if (isdigit((unsigned char)c)) {
				string lex(1, c);
				while (pos < src.size() && isdigit((unsigned char)peek()))
					lex += advance();
				return { TokenType::NUMBER, lex, sl, sc };
			}

			// Handle Identifiers and Reserved Keywords
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

			// Handle Operators and Punctuation
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
				return { TokenType::LEXERROR, ":",  sl, sc };
			}
			return { TokenType::LEXERROR, string(1, c), sl, sc };
		}

		// Generates the full list of tokens for the parser
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
	//  PARSE ERRORS (Phase 2)
	//  Distinguishes between Lexical Errors (invalid chars) 
	//  and Syntax Errors (grammar violations).
	// ================================================================
	enum class ErrorKind { LEXICAL, SYNTAX };

	struct ParseError {
		ErrorKind   kind;
		string      message;
		int         line;
		int         col;
	};

	// ================================================================
	//  AST NODE (Phase 2)
	//  Structure to represent the Abstract Syntax Tree.
	// ================================================================
	enum class NodeKind {
		PROGRAM, STMT_SEQ,
		IF_STMT, REPEAT_STMT, ASSIGN_STMT, READ_STMT, WRITE_STMT,
		EXP, SIMPLE_EXP, TERM, FACTOR,
		ID_NODE, NUM_NODE, STRING_NODE, OP_NODE
	};

	struct ASTNode {
		NodeKind          kind;
		string            value;
		int               line = 0;
		vector<ASTNode*>  children;

		ASTNode(NodeKind k, string v = "", int ln = 0)
			: kind(k), value(v), line(ln) {
		}

		~ASTNode() {
			for (auto* c : children) delete c;
		}

		void addChild(ASTNode* c) { if (c) children.push_back(c); }
	};

	// ================================================================
	//  PARSER CLASS (Phase 2)
	//  Top-Down Recursive Descent Parser to validate the Grammar
	//  and build the Abstract Syntax Tree (AST).
	// ================================================================
	class Parser {
		const vector<Token>& tokens;
		size_t               pos = 0;
		vector<ParseError>   errors;

		const Token& current() const {
			return (pos < tokens.size()) ? tokens[pos] : tokens.back();
		}
		const Token& peek(size_t offset = 1) const {
			size_t i = pos + offset;
			return (i < tokens.size()) ? tokens[i] : tokens.back();
		}
		bool atEnd() const { return current().type == TokenType::ENDOFFILE; }
		bool check(TokenType t) const { return current().type == t; }

		// Enforce expected tokens. If mismatch, log a syntax error.
		bool expect(TokenType t, const string& expected) {
			if (check(t)) {
				pos++;
				return true;
			}
			const Token& cur = current();
			ostringstream oss;
			oss << "Expected '" << expected << "' but found '" << cur.lexeme << "'";
			errors.push_back({ ErrorKind::SYNTAX, oss.str(), cur.line, cur.col });
			return false;
		}

		const Token& consume() {
			const Token& t = current();
			if (!atEnd()) pos++;
			return t;
		}

		// Error recovery mechanism to prevent cascaded errors
		void synchronise() {
			while (!atEnd()) {
				TokenType t = current().type;
				if (t == TokenType::SEMICOLON ||
					t == TokenType::END_KW ||
					t == TokenType::ELSE_KW ||
					t == TokenType::UNTIL_KW)
					return;
				pos++;
			}
		}

		bool isStartOfStatement() const {
			switch (current().type) {
			case TokenType::IF_KW:
			case TokenType::REPEAT_KW:
			case TokenType::ID:
			case TokenType::READ_KW:
			case TokenType::WRITE_KW: return true;
			default: return false;
			}
		}

		// Base Grammar Rules
		ASTNode* parseProgram() {
			ASTNode* node = new ASTNode(NodeKind::PROGRAM, "program");
			node->addChild(parseStmtSequence());
			if (!atEnd()) {
				const Token& cur = current();
				ostringstream oss;
				oss << "Unexpected token '" << cur.lexeme << "' after end of program";
				errors.push_back({ ErrorKind::SYNTAX, oss.str(), cur.line, cur.col });
			}
			return node;
		}

		ASTNode* parseStmtSequence() {
			ASTNode* node = new ASTNode(NodeKind::STMT_SEQ, "stmt-seq");
			if (!isStartOfStatement()) {
				const Token& cur = current();
				ostringstream oss;
				oss << "Expected a statement (if/repeat/id/read/write) but found '" << cur.lexeme << "'";
				errors.push_back({ ErrorKind::SYNTAX, oss.str(), cur.line, cur.col });
				synchronise();
				return node;
			}
			node->addChild(parseStatement());
			parseStmtSequencePrime(node);
			return node;
		}

		void parseStmtSequencePrime(ASTNode* parent) {
			while (check(TokenType::SEMICOLON)) {
				pos++;
				if (!isStartOfStatement()) break;
				parent->addChild(parseStatement());
			}
		}

		ASTNode* parseStatement() {
			// Handle Lexical Errors separately in the parsing stage
			if (check(TokenType::LEXERROR)) {
				const Token& cur = current();
				ostringstream oss;
				oss << "Lexical error: unrecognized token '" << cur.lexeme << "'";
				errors.push_back({ ErrorKind::LEXICAL, oss.str(), cur.line, cur.col });
				pos++;
				return new ASTNode(NodeKind::STMT_SEQ, "error-skip", current().line);
			}

			switch (current().type) {
			case TokenType::IF_KW:     return parseIfStmt();
			case TokenType::REPEAT_KW: return parseRepeatStmt();
			case TokenType::ID:        return parseAssignStmt();
			case TokenType::READ_KW:   return parseReadStmt();
			case TokenType::WRITE_KW:  return parseWriteStmt();
			default: {
				const Token& cur = current();
				ostringstream oss;
				oss << "Unexpected token '" << cur.lexeme << "'; expected a statement";
				errors.push_back({ ErrorKind::SYNTAX, oss.str(), cur.line, cur.col });
				synchronise();
				return new ASTNode(NodeKind::STMT_SEQ, "error", cur.line);
			}
			}
		}

		ASTNode* parseIfStmt() {
			int ln = current().line;
			ASTNode* node = new ASTNode(NodeKind::IF_STMT, "if", ln);
			expect(TokenType::IF_KW, "if");
			node->addChild(parseExp());
			expect(TokenType::THEN_KW, "then");
			node->addChild(parseStmtSequence());

			if (check(TokenType::ELSE_KW)) {
				pos++;
				node->addChild(parseStmtSequence());
				expect(TokenType::END_KW, "end");
			}
			else {
				expect(TokenType::END_KW, "end");
			}
			return node;
		}

		ASTNode* parseRepeatStmt() {
			int ln = current().line;
			ASTNode* node = new ASTNode(NodeKind::REPEAT_STMT, "repeat", ln);
			expect(TokenType::REPEAT_KW, "repeat");
			node->addChild(parseStmtSequence());
			expect(TokenType::UNTIL_KW, "until");
			node->addChild(parseExp());
			return node;
		}

		ASTNode* parseAssignStmt() {
			int ln = current().line;
			string name = current().lexeme;
			ASTNode* node = new ASTNode(NodeKind::ASSIGN_STMT, name, ln);
			expect(TokenType::ID, "identifier");
			expect(TokenType::ASSIGNMENTOP, ":=");
			node->addChild(parseExp());
			return node;
		}

		ASTNode* parseReadStmt() {
			int ln = current().line;
			ASTNode* node = new ASTNode(NodeKind::READ_STMT, "read", ln);
			expect(TokenType::READ_KW, "read");
			node->addChild(parseIdList());
			return node;
		}

		ASTNode* parseWriteStmt() {
			int ln = current().line;
			ASTNode* node = new ASTNode(NodeKind::WRITE_STMT, "write", ln);
			expect(TokenType::WRITE_KW, "write");
			node->addChild(parseExpList());
			return node;
		}

		ASTNode* parseIdList() {
			ASTNode* node = new ASTNode(NodeKind::STMT_SEQ, "id-list");
			if (!check(TokenType::ID)) {
				const Token& cur = current();
				ostringstream oss;
				oss << "Expected identifier after 'read' but found '" << cur.lexeme << "'";
				errors.push_back({ ErrorKind::SYNTAX, oss.str(), cur.line, cur.col });
				return node;
			}
			node->addChild(new ASTNode(NodeKind::ID_NODE, current().lexeme, current().line));
			pos++;
			while (check(TokenType::COMMA)) {
				pos++;
				if (!check(TokenType::ID)) {
					const Token& cur = current();
					ostringstream oss;
					oss << "Expected identifier after ',' in read list but found '" << cur.lexeme << "'";
					errors.push_back({ ErrorKind::SYNTAX, oss.str(), cur.line, cur.col });
					break;
				}
				node->addChild(new ASTNode(NodeKind::ID_NODE, current().lexeme, current().line));
				pos++;
			}
			return node;
		}

		ASTNode* parseExpList() {
			ASTNode* node = new ASTNode(NodeKind::STMT_SEQ, "exp-list");
			node->addChild(parseExpItem());
			while (check(TokenType::COMMA)) {
				pos++;
				node->addChild(parseExpItem());
			}
			return node;
		}

		ASTNode* parseExpItem() {
			if (check(TokenType::STRING_LIT)) {
				ASTNode* n = new ASTNode(NodeKind::STRING_NODE, current().lexeme, current().line);
				pos++;
				return n;
			}
			return parseExp();
		}

		ASTNode* parseExp() {
			ASTNode* left = parseSimpleExp();
			while (check(TokenType::COMPARISONOP)) {
				string op = current().lexeme;
				int ln = current().line;
				pos++;
				ASTNode* node = new ASTNode(NodeKind::EXP, op, ln);
				node->addChild(left);
				node->addChild(parseSimpleExp());
				left = node;
			}
			return left;
		}

		ASTNode* parseSimpleExp() {
			ASTNode* left = parseTerm();
			while (check(TokenType::ADDOP) || check(TokenType::SUBOP)) {
				string op = current().lexeme;
				int ln = current().line;
				pos++;
				ASTNode* node = new ASTNode(NodeKind::SIMPLE_EXP, op, ln);
				node->addChild(left);
				node->addChild(parseTerm());
				left = node;
			}
			return left;
		}

		ASTNode* parseTerm() {
			ASTNode* left = parseFactor();
			while (check(TokenType::MULOP) || check(TokenType::DIVOP)) {
				string op = current().lexeme;
				int ln = current().line;
				pos++;
				ASTNode* node = new ASTNode(NodeKind::TERM, op, ln);
				node->addChild(left);
				node->addChild(parseFactor());
				left = node;
			}
			return left;
		}

		ASTNode* parseFactor() {
			if (check(TokenType::PUNCTUATION) && current().lexeme == "(") {
				pos++;
				ASTNode* node = parseExp();
				expect(TokenType::PUNCTUATION, ")");
				return node;
			}
			if (check(TokenType::NUMBER)) {
				ASTNode* n = new ASTNode(NodeKind::NUM_NODE, current().lexeme, current().line);
				pos++;
				return n;
			}
			if (check(TokenType::ID)) {
				ASTNode* n = new ASTNode(NodeKind::ID_NODE, current().lexeme, current().line);
				pos++;
				return n;
			}
			const Token& cur = current();
			ostringstream oss;
			oss << "Expected an expression (identifier, number, or '(') but found '" << cur.lexeme << "'";
			errors.push_back({ ErrorKind::SYNTAX, oss.str(), cur.line, cur.col });
			ASTNode* dummy = new ASTNode(NodeKind::FACTOR, "?", cur.line);
			if (!atEnd()) pos++;
			return dummy;
		}

	public:
		explicit Parser(const vector<Token>& toks) : tokens(toks) {}

		// Execution entry point. Returns the root of AST and any captured errors.
		pair<ASTNode*, vector<ParseError>> parse() {
			pos = 0;
			errors.clear();
			ASTNode* root = parseProgram();
			return { root, errors };
		}
	};

	// ================================================================
	//  WINDOWS FORM GUI (Phase 1 + Phase 2)
	//  Maintains the presentation layer, separating Logic from UI.
	// ================================================================
	public ref class Form1 : public Form {

		RichTextBox^ txtSource;
		DataGridView^ dgvTokens;
		Button^ btnScan;
		Button^ btnClear;
		Label^ lblInput;
		Label^ lblStatus;
		Panel^ pnlHeader;
		Label^ lblTitle;

		TabControl^ tabResults;
		TabPage^ tabLexer;
		TabPage^ tabParser;
		SplitContainer^ splitParser;
		TreeView^ tvAST;
		DataGridView^ dgvErrors;
		Label^ lblParseResult;
		Button^ btnScanParse;

	public:
		Form1() { InitializeComponent(); }
	protected:
		~Form1() {}

	private:

		void addTokenCol(String^ header, String^ name, int fw, DataGridViewContentAlignment align) {
			if (!dgvTokens) return;
			DataGridViewTextBoxColumn^ col = gcnew DataGridViewTextBoxColumn();
			col->HeaderText = header;
			col->Name = name;
			col->FillWeight = (float)fw;
			col->DefaultCellStyle->Alignment = align;
			col->SortMode = DataGridViewColumnSortMode::NotSortable;
			dgvTokens->Columns->Add(col);
		}

		void addErrorCol(String^ header, String^ name, int fw, DataGridViewContentAlignment align) {
			if (!dgvErrors) return;
			DataGridViewTextBoxColumn^ col = gcnew DataGridViewTextBoxColumn();
			col->HeaderText = header;
			col->Name = name;
			col->FillWeight = (float)fw;
			col->DefaultCellStyle->Alignment = align;
			col->SortMode = DataGridViewColumnSortMode::NotSortable;
			dgvErrors->Columns->Add(col);
		}

		void InitializeComponent() {
			this->Text = L"Tiny Language - Lexical & Syntax Analyzer (Phase 1 + 2)";
			this->Size = Drawing::Size(1150, 820);
			this->MinimumSize = Drawing::Size(900, 700);
			this->StartPosition = FormStartPosition::CenterScreen;
			this->BackColor = Color::FromArgb(240, 244, 248);
			this->Font = gcnew Drawing::Font("Segoe UI", 9.5f);

			pnlHeader = gcnew Panel();
			pnlHeader->Dock = DockStyle::Top;
			pnlHeader->Height = 60;
			pnlHeader->BackColor = Color::FromArgb(20, 45, 90);

			lblTitle = gcnew Label();
			lblTitle->Text = L"TINY Language - Lexical & Syntax Analyzer  |  Phase 1 + Phase 2";
			lblTitle->ForeColor = Color::White;
			lblTitle->Font = gcnew Drawing::Font("Segoe UI", 13.0f, FontStyle::Bold);
			lblTitle->Dock = DockStyle::Fill;
			lblTitle->TextAlign = ContentAlignment::MiddleLeft;
			lblTitle->Padding = System::Windows::Forms::Padding(14, 0, 0, 0);
			pnlHeader->Controls->Add(lblTitle);

			lblInput = gcnew Label();
			lblInput->Text = L"Source Code Input:";
			lblInput->Font = gcnew Drawing::Font("Segoe UI", 10.0f, FontStyle::Bold);
			lblInput->ForeColor = Color::FromArgb(20, 45, 90);
			lblInput->Location = Point(10, 70);
			lblInput->Size = Drawing::Size(300, 22);

			txtSource = gcnew RichTextBox();
			txtSource->Location = Point(10, 95);
			txtSource->Size = Drawing::Size(1115, 185);
			txtSource->Font = gcnew Drawing::Font("Consolas", 10.5f);
			txtSource->BackColor = Color::FromArgb(30, 30, 46);
			txtSource->ForeColor = Color::FromArgb(202, 211, 245);
			txtSource->BorderStyle = BorderStyle::FixedSingle;
			txtSource->ScrollBars = RichTextBoxScrollBars::Both;
			txtSource->WordWrap = false;

			// Cleaned Up String Literal (Removed underscores representing hyphens)
			txtSource->Text =
				L"{ Sample Tiny program - computes factorial }\r\n"
				L"read x;\r\n"
				L"if 0 < x then\r\n"
				L"    fact := 1;\r\n"
				L"    repeat\r\n"
				L"        fact := fact * x;\r\n"
				L"        x := x - 1\r\n"
				L"    until x = 0;\r\n"
				L"    write fact\r\n"
				L"end";

			btnScanParse = gcnew Button();
			btnScanParse->Text = L"Scan + Parse";
			btnScanParse->Location = Point(10, 292);
			btnScanParse->Size = Drawing::Size(155, 40);
			btnScanParse->BackColor = Color::FromArgb(20, 45, 90);
			btnScanParse->ForeColor = Color::White;
			btnScanParse->FlatStyle = FlatStyle::Flat;
			btnScanParse->FlatAppearance->BorderSize = 0;
			btnScanParse->Font = gcnew Drawing::Font("Segoe UI", 10.5f, FontStyle::Bold);
			btnScanParse->Cursor = Cursors::Hand;
			btnScanParse->Click += gcnew EventHandler(this, &Form1::btnScanParse_Click);

			btnScan = gcnew Button();
			btnScan->Text = L"Scan Only";
			btnScan->Location = Point(175, 292);
			btnScan->Size = Drawing::Size(130, 40);
			btnScan->BackColor = Color::FromArgb(60, 90, 150);
			btnScan->ForeColor = Color::White;
			btnScan->FlatStyle = FlatStyle::Flat;
			btnScan->FlatAppearance->BorderSize = 0;
			btnScan->Font = gcnew Drawing::Font("Segoe UI", 10.0f, FontStyle::Bold);
			btnScan->Cursor = Cursors::Hand;
			btnScan->Click += gcnew EventHandler(this, &Form1::btnScan_Click);

			btnClear = gcnew Button();
			btnClear->Text = L"Clear";
			btnClear->Location = Point(315, 292);
			btnClear->Size = Drawing::Size(110, 40);
			btnClear->BackColor = Color::FromArgb(160, 55, 55);
			btnClear->ForeColor = Color::White;
			btnClear->FlatStyle = FlatStyle::Flat;
			btnClear->FlatAppearance->BorderSize = 0;
			btnClear->Font = gcnew Drawing::Font("Segoe UI", 10.0f, FontStyle::Bold);
			btnClear->Cursor = Cursors::Hand;
			btnClear->Click += gcnew EventHandler(this, &Form1::btnClear_Click);

			lblParseResult = gcnew Label();
			lblParseResult->Text = L"";
			lblParseResult->Location = Point(440, 298);
			lblParseResult->Size = Drawing::Size(680, 28);
			lblParseResult->Font = gcnew Drawing::Font("Segoe UI", 11.0f, FontStyle::Bold);
			lblParseResult->ForeColor = Color::Gray;

			tabResults = gcnew TabControl();
			tabResults->Location = Point(10, 342);
			tabResults->Size = Drawing::Size(1115, 410);
			tabResults->Font = gcnew Drawing::Font("Segoe UI", 10.0f);

			tabLexer = gcnew TabPage();
			tabLexer->Text = L"  Token Table (Lexer)  ";
			tabLexer->BackColor = Color::White;

			dgvTokens = gcnew DataGridView();
			dgvTokens->Dock = DockStyle::Fill;
			dgvTokens->ReadOnly = true;
			dgvTokens->AllowUserToAddRows = false;
			dgvTokens->AllowUserToDeleteRows = false;
			dgvTokens->RowHeadersVisible = false;
			dgvTokens->SelectionMode = DataGridViewSelectionMode::FullRowSelect;
			dgvTokens->AutoSizeColumnsMode = DataGridViewAutoSizeColumnsMode::Fill;
			dgvTokens->BackgroundColor = Color::White;
			dgvTokens->BorderStyle = BorderStyle::None;
			dgvTokens->GridColor = Color::FromArgb(210, 215, 225);
			dgvTokens->Font = gcnew Drawing::Font("Consolas", 10.0f);
			dgvTokens->ColumnHeadersHeight = 36;
			dgvTokens->EnableHeadersVisualStyles = false;
			dgvTokens->ColumnHeadersDefaultCellStyle->Font = gcnew Drawing::Font("Segoe UI", 10.0f, FontStyle::Bold);
			dgvTokens->ColumnHeadersDefaultCellStyle->BackColor = Color::FromArgb(20, 45, 90);
			dgvTokens->ColumnHeadersDefaultCellStyle->ForeColor = Color::White;
			dgvTokens->ColumnHeadersDefaultCellStyle->Alignment = DataGridViewContentAlignment::MiddleCenter;
			dgvTokens->AlternatingRowsDefaultCellStyle->BackColor = Color::FromArgb(245, 248, 255);
			dgvTokens->CellFormatting += gcnew DataGridViewCellFormattingEventHandler(this, &Form1::dgvTokens_CellFormatting);

			addTokenCol(L"#", L"colIdx", 7, DataGridViewContentAlignment::MiddleCenter);
			addTokenCol(L"Token Type", L"colType", 22, DataGridViewContentAlignment::MiddleCenter);
			addTokenCol(L"Lexeme", L"colLexeme", 32, DataGridViewContentAlignment::MiddleLeft);
			addTokenCol(L"Line", L"colLine", 7, DataGridViewContentAlignment::MiddleCenter);
			addTokenCol(L"Column", L"colCol", 7, DataGridViewContentAlignment::MiddleCenter);

			tabLexer->Controls->Add(dgvTokens);

			tabParser = gcnew TabPage();
			tabParser->Text = L"  Syntax Analysis (Parser)  ";
			tabParser->BackColor = Color::FromArgb(248, 248, 252);

			// ==========================================
			// UI Enhancement 1: Horizontal SplitContainer
			// ==========================================
			splitParser = gcnew SplitContainer();
			splitParser->Dock = DockStyle::Fill;
			splitParser->Orientation = Orientation::Horizontal; // AST on Top, Errors on Bottom
			splitParser->SplitterDistance = 240;
			splitParser->Panel1->BackColor = Color::FromArgb(30, 30, 46);
			splitParser->Panel2->BackColor = Color::White;
			splitParser->Panel2Collapsed = true; // Auto-Hide error panel initially

			tvAST = gcnew TreeView();
			tvAST->Dock = DockStyle::Fill;
			tvAST->Font = gcnew Drawing::Font("Consolas", 11.5f);
			tvAST->BackColor = Color::FromArgb(30, 30, 46);
			tvAST->ForeColor = Color::FromArgb(166, 218, 149);
			tvAST->BorderStyle = BorderStyle::None;
			splitParser->Panel1->Controls->Add(tvAST);

			dgvErrors = gcnew DataGridView();
			dgvErrors->Dock = DockStyle::Fill;
			dgvErrors->ReadOnly = true;
			dgvErrors->AllowUserToAddRows = false;
			dgvErrors->AllowUserToDeleteRows = false;
			dgvErrors->RowHeadersVisible = false;
			dgvErrors->SelectionMode = DataGridViewSelectionMode::FullRowSelect;
			dgvErrors->AutoSizeColumnsMode = DataGridViewAutoSizeColumnsMode::Fill;

			// ==========================================
			// UI Enhancement 2: Dynamic Row Height (Word Wrap)
			// ==========================================
			dgvErrors->AutoSizeRowsMode = DataGridViewAutoSizeRowsMode::AllCells;
			dgvErrors->DefaultCellStyle->WrapMode = DataGridViewTriState::True;

			dgvErrors->BackgroundColor = Color::White;
			dgvErrors->BorderStyle = BorderStyle::None;
			dgvErrors->GridColor = Color::FromArgb(210, 215, 225);
			dgvErrors->Font = gcnew Drawing::Font("Segoe UI", 10.0f);
			dgvErrors->ColumnHeadersHeight = 36;
			dgvErrors->EnableHeadersVisualStyles = false;
			dgvErrors->ColumnHeadersDefaultCellStyle->Font = gcnew Drawing::Font("Segoe UI", 10.0f, FontStyle::Bold);
			dgvErrors->ColumnHeadersDefaultCellStyle->BackColor = Color::FromArgb(180, 40, 40);
			dgvErrors->ColumnHeadersDefaultCellStyle->ForeColor = Color::White;
			dgvErrors->ColumnHeadersDefaultCellStyle->Alignment = DataGridViewContentAlignment::MiddleCenter;
			dgvErrors->AlternatingRowsDefaultCellStyle->BackColor = Color::FromArgb(255, 245, 245);
			dgvErrors->CellFormatting += gcnew DataGridViewCellFormattingEventHandler(this, &Form1::dgvErrors_CellFormatting);

			addErrorCol(L"#", L"eIdx", 6, DataGridViewContentAlignment::MiddleCenter);
			addErrorCol(L"Error Type", L"eKind", 18, DataGridViewContentAlignment::MiddleCenter);
			addErrorCol(L"Line", L"eLine", 8, DataGridViewContentAlignment::MiddleCenter);
			addErrorCol(L"Column", L"eCol", 8, DataGridViewContentAlignment::MiddleCenter);
			addErrorCol(L"Message", L"eMsg", 60, DataGridViewContentAlignment::MiddleLeft);

			splitParser->Panel2->Controls->Add(dgvErrors);
			tabParser->Controls->Add(splitParser);

			tabResults->TabPages->Add(tabLexer);
			tabResults->TabPages->Add(tabParser);

			lblStatus = gcnew Label();
			lblStatus->Text = L"Ready. Enter Tiny source code above.";
			lblStatus->Location = Point(10, 762);
			lblStatus->Size = Drawing::Size(1115, 22);
			lblStatus->ForeColor = Color::FromArgb(80, 100, 130);
			lblStatus->Font = gcnew Drawing::Font("Segoe UI", 9.0f, FontStyle::Italic);

			this->Controls->Add(pnlHeader);
			this->Controls->Add(lblInput);
			this->Controls->Add(txtSource);
			this->Controls->Add(btnScanParse);
			this->Controls->Add(btnScan);
			this->Controls->Add(btnClear);
			this->Controls->Add(lblParseResult);
			this->Controls->Add(tabResults);
			this->Controls->Add(lblStatus);

			this->Resize += gcnew EventHandler(this, &Form1::Form1_Resize);
		}

		void Form1_Resize(Object^ sender, EventArgs^ e) {
			int w = this->ClientSize.Width - 20;
			int h = this->ClientSize.Height;
			txtSource->Width = w;
			tabResults->Width = w;
			lblStatus->Width = w;
			lblParseResult->Width = w - 440;
			lblStatus->Location = Point(10, h - 30);
			int tabH = h - tabResults->Location.Y - 40;
			if (tabH > 100) tabResults->Height = tabH;
		}

		// Recursively maps AST nodes into the TreeView UI element
		void populateTree(ASTNode* node, TreeNode^ uiNode) {
			if (!node) return;
			for (auto child : node->children) {
				if (child) {
					String^ text = "";
					switch (child->kind) {
					case NodeKind::PROGRAM:      text = "[program]"; break;
					case NodeKind::STMT_SEQ:     text = "[" + gcnew String(child->value.c_str()) + "]"; break;
					case NodeKind::IF_STMT:      text = "[if-stmt]"; break;
					case NodeKind::REPEAT_STMT:  text = "[repeat-stmt]"; break;
					case NodeKind::ASSIGN_STMT:  text = "[assign: " + gcnew String(child->value.c_str()) + "]"; break;
					case NodeKind::READ_STMT:    text = "[read]"; break;
					case NodeKind::WRITE_STMT:   text = "[write]"; break;
					case NodeKind::EXP:          text = "[cmp: " + gcnew String(child->value.c_str()) + "]"; break;
					case NodeKind::SIMPLE_EXP:   text = "[add: " + gcnew String(child->value.c_str()) + "]"; break;
					case NodeKind::TERM:         text = "[mul: " + gcnew String(child->value.c_str()) + "]"; break;
					case NodeKind::FACTOR:       text = "[factor: " + gcnew String(child->value.c_str()) + "]"; break;
					case NodeKind::ID_NODE:      text = "id(" + gcnew String(child->value.c_str()) + ")"; break;
					case NodeKind::NUM_NODE:     text = "num(" + gcnew String(child->value.c_str()) + ")"; break;
					case NodeKind::STRING_NODE:  text = "str(" + gcnew String(child->value.c_str()) + ")"; break;
					case NodeKind::OP_NODE:      text = "op(" + gcnew String(child->value.c_str()) + ")"; break;
					}
					TreeNode^ newUiNode = gcnew TreeNode(text);
					uiNode->Nodes->Add(newUiNode);
					populateTree(child, newUiNode);
				}
			}
		}

		// Invokes Lexer logic and Populates Token Grid
		vector<Token> runLexer(const string& src) {
			dgvTokens->Rows->Clear();
			Lexer lexer(src);
			vector<Token> tokens = lexer.tokenize();
			int idx = 1, lexErrCount = 0;
			for (const Token& tok : tokens) {
				if (tok.type == TokenType::ENDOFFILE) break;
				String^ typeStr = tokenName(tok.type);
				String^ lexStr = gcnew String(tok.lexeme.c_str());
				String^ lineStr = gcnew String(to_string(tok.line).c_str());
				String^ colStr = gcnew String(to_string(tok.col).c_str());
				String^ idxStr = gcnew String(to_string(idx).c_str());
				dgvTokens->Rows->Add(idxStr, typeStr, lexStr, lineStr, colStr);
				if (tok.type == TokenType::LEXERROR) lexErrCount++;
				idx++;
			}
			return tokens;
		}

		// Triggered upon pressing "Scan Only"
		void btnScan_Click(Object^ sender, EventArgs^ e) {
			String^ managed = txtSource->Text;
			if (managed->Trim()->Length == 0) {
				lblStatus->Text = L"Input is empty.";
				lblStatus->ForeColor = Color::OrangeRed;
				return;
			}
			string src = msclr::interop::marshal_as<string>(managed);
			vector<Token> tokens = runLexer(src);

			int total = (int)dgvTokens->Rows->Count;
			int errs = 0;
			for (const Token& t : tokens)
				if (t.type == TokenType::LEXERROR) errs++;

			if (errs > 0) {
				lblStatus->Text = String::Format(L"Lexer: {0} token(s), {1} lexical error(s).", total, errs);
				lblStatus->ForeColor = Color::OrangeRed;
			}
			else {
				lblStatus->Text = String::Format(L"Lexer: {0} token(s) found. No lexical errors.", total);
				lblStatus->ForeColor = Color::FromArgb(0, 120, 70);
			}
			tabResults->SelectedTab = tabLexer;
			lblParseResult->Text = L"";
		}

		// Triggered upon pressing "Scan + Parse"
		void btnScanParse_Click(Object^ sender, EventArgs^ e) {
			String^ managed = txtSource->Text;
			if (managed->Trim()->Length == 0) {
				lblStatus->Text = L"Input is empty.";
				lblStatus->ForeColor = Color::OrangeRed;
				return;
			}
			string src = msclr::interop::marshal_as<string>(managed);

			vector<Token> tokens = runLexer(src);
			int lexTotal = (int)dgvTokens->Rows->Count;

			dgvErrors->Rows->Clear();
			tvAST->Nodes->Clear();

			Parser parser(tokens);
			pair<ASTNode*, vector<ParseError>> result = parser.parse();
			ASTNode* root = result.first;
			vector<ParseError> parseErrors = result.second;

			if (root) {
				TreeNode^ rootUiNode = gcnew TreeNode("[program]");
				tvAST->Nodes->Add(rootUiNode);
				populateTree(root, rootUiNode);
				tvAST->ExpandAll();
				delete root;
			}
			else {
				tvAST->Nodes->Add("Failed to parse AST.");
			}

			int errIdx = 1;
			for (const ParseError& pe : parseErrors) {
				String^ kindStr = (pe.kind == ErrorKind::LEXICAL) ? L"Lexical Error" : L"Syntax Error";
				String^ lineStr = gcnew String(to_string(pe.line).c_str());
				String^ colStr = gcnew String(to_string(pe.col).c_str());
				String^ msgStr = gcnew String(pe.message.c_str());
				String^ idxStr = gcnew String(to_string(errIdx++).c_str());
				dgvErrors->Rows->Add(idxStr, kindStr, lineStr, colStr, msgStr);
			}

			int totalErrors = (int)parseErrors.size();

			// ==========================================
			// UI Enhancement 3: Auto-Hide Error Table if Code is Clean
			// ==========================================
			if (totalErrors == 0) {
				splitParser->Panel2Collapsed = true; // Collapse Errors Panel, Expand AST
				lblParseResult->Text = L"Syntax Analysis PASSED  ✓  No errors detected.";
				lblParseResult->ForeColor = Color::FromArgb(0, 130, 60);
				lblStatus->Text = String::Format(L"Scan + Parse complete - {0} token(s), 0 errors.", lexTotal);
				lblStatus->ForeColor = Color::FromArgb(0, 130, 60);
			}
			else {
				splitParser->Panel2Collapsed = false; // Reveal Errors Panel
				int synErrs = 0, lexErrs = 0;
				for (const ParseError& pe : parseErrors) {
					if (pe.kind == ErrorKind::SYNTAX) synErrs++;
					else lexErrs++;
				}
				const ParseError& first = parseErrors[0];
				String^ firstMsg = gcnew String(first.message.c_str());
				String^ kindLabel = (first.kind == ErrorKind::LEXICAL) ? L"Lexical Error" : L"Syntax Error";
				lblParseResult->Text = String::Format(L"{0} at Line {1}: {2}  ({3} total error(s))", kindLabel, first.line, firstMsg, totalErrors);
				lblParseResult->ForeColor = Color::FromArgb(200, 40, 40);
				lblStatus->Text = String::Format(L"Scan + Parse - {0} token(s) | {1} Syntax Error(s), {2} Lexical Error(s).", lexTotal, synErrs, lexErrs);
				lblStatus->ForeColor = Color::OrangeRed;
			}

			tabResults->SelectedTab = (totalErrors > 0) ? tabParser : tabLexer;
		}

		// Wipes all data across fields and tabs
		void btnClear_Click(Object^ sender, EventArgs^ e) {
			txtSource->Clear();
			dgvTokens->Rows->Clear();
			dgvErrors->Rows->Clear();
			tvAST->Nodes->Clear();
			splitParser->Panel2Collapsed = true; // Hide error panel upon clear
			lblParseResult->Text = L"";
			lblStatus->Text = L"Cleared.";
			lblStatus->ForeColor = Color::FromArgb(80, 100, 130);
		}

		// Dynamically style token row colors based on Token Type
		void dgvTokens_CellFormatting(Object^ sender, DataGridViewCellFormattingEventArgs^ e) {
			if (e->RowIndex < 0) return;
			DataGridViewRow^ row = dgvTokens->Rows[e->RowIndex];
			String^ type = safe_cast<String^>(row->Cells["colType"]->Value);
			if (!type) return;
			if (type->EndsWith("_KW")) {
				row->DefaultCellStyle->BackColor = Color::FromArgb(220, 235, 255);
				row->DefaultCellStyle->ForeColor = Color::FromArgb(10, 50, 150);
			}
			else if (type == "ID") {
				row->DefaultCellStyle->BackColor = Color::FromArgb(220, 255, 230);
				row->DefaultCellStyle->ForeColor = Color::FromArgb(0, 100, 40);
			}
			else if (type == "NUMBER") {
				row->DefaultCellStyle->BackColor = Color::FromArgb(255, 245, 215);
				row->DefaultCellStyle->ForeColor = Color::FromArgb(150, 80, 0);
			}
			else if (type == "STRING") {
				row->DefaultCellStyle->BackColor = Color::FromArgb(245, 220, 255);
				row->DefaultCellStyle->ForeColor = Color::FromArgb(100, 0, 150);
			}
			else if (type == "ADDOP" || type == "SUBOP" || type == "MULOP" || type == "DIVOP" || type == "COMPARISONOP" || type == "ASSIGNMENTOP") {
				row->DefaultCellStyle->BackColor = Color::FromArgb(255, 255, 210);
				row->DefaultCellStyle->ForeColor = Color::FromArgb(120, 100, 0);
			}
			else if (type == "LEXERROR") {
				row->DefaultCellStyle->BackColor = Color::FromArgb(255, 210, 210);
				row->DefaultCellStyle->ForeColor = Color::FromArgb(180, 0, 0);
			}
		}

		// Dynamically style error rows based on Severity/Kind
		void dgvErrors_CellFormatting(Object^ sender, DataGridViewCellFormattingEventArgs^ e) {
			if (e->RowIndex < 0) return;
			DataGridViewRow^ row = dgvErrors->Rows[e->RowIndex];
			String^ kind = safe_cast<String^>(row->Cells["eKind"]->Value);
			if (!kind) return;
			if (kind == "Lexical Error") {
				row->DefaultCellStyle->BackColor = Color::FromArgb(255, 225, 225);
				row->DefaultCellStyle->ForeColor = Color::FromArgb(160, 0, 0);
			}
			else {
				row->DefaultCellStyle->BackColor = Color::FromArgb(255, 240, 210);
				row->DefaultCellStyle->ForeColor = Color::FromArgb(140, 70, 0);
			}
		}
	};
}