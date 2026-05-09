# Tiny Language — Lexical & Syntax Analyzer
### Phase 1 (Lexer) + Phase 2 (Parser)

A fully interactive C++/CLI Windows Forms desktop application that implements both the Lexical Analysis and Syntax Analysis phases of a compiler for the Tiny educational programming language.

## Overview
This project implements the first two phases of a complete compiler pipeline:

| Phase | Component | Output |
|-------|-----------|--------|
| 1 | Lexical Analyzer | Colored token table |
| 2 | Syntax Analyzer | Parse tree + error table |

The architecture deliberately separates concerns:
* **Lexer and Parser** are pure standard C++ classes — zero CLR dependencies.
* **Form1** is the C++/CLI presentation layer only — it calls the engine and displays results.

## Features

### Phase 1 — Lexer
* Full Tiny language token support (8 keywords, ID, NUMBER, STRING, 6 operators, 5 symbols)
* Exact line + column tracking for every token
* `{ ... }` comments silently consumed
* Color-coded token table (blue=keywords, green=ID, orange=numbers, purple=strings, yellow=operators, red=errors)
* Lexical errors reported as `LEXERROR` tokens with precise location

### Phase 2 — Parser
* Recursive Descent Parser based on the LL(1) grammar
* Full grammar coverage: `if/then/else/end`, `repeat/until`, assignments, `read`/`write`
* Expression parsing with correct operator precedence: `*,/` > `+,-` > `=,<`
* **AST (Abstract Syntax Tree) displayed as an interactive, expandable TreeView UI** (Updated)
* **Smart UI Layout: Horizontal split showing the AST on top, and an auto-hiding Error Table on the bottom** (Updated)
* Two error types clearly distinguished:
  * **Lexical Error** — unrecognised character or malformed token
  * **Syntax Error** — wrong token sequence, missing keyword, etc.
* Every error shows: Line Number, Column, Error Type, Detailed Message
* **Error recovery via synchronisation** — parser continues after an error to find more
* Parse result banner: green "PASSED" or red "Error at Line X: ..."

## GUI Layout
```text
┌─────────────────────────────────────────────────────────────────────┐
│  TINY Language - Lexical & Syntax Analyzer  |  Phase 1 + Phase 2    │  ← header
├─────────────────────────────────────────────────────────────────────┤
│  Source Code Input:                                                 │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │  { dark-themed code editor — Consolas font }                  │  │  ← RichTextBox
│  │  read x;                                                      │  │
│  │  if 0 < x then ...                                            │  │
│  └───────────────────────────────────────────────────────────────┘  │
│  [Scan + Parse]  [Scan Only]  [Clear]   Syntax PASSED ✓             │  ← buttons + banner
├─────────────────────────────────────────────────────────────────────┤
│  ┌── Token Table (Lexer) ──────┐  ┌── Syntax Analysis (Parser) ──┐  │
│  │  # | Type | Lexeme | Ln|Col │  │ [Interactive TreeView (Top)] │  │  ← TabControl
│  │  (color-coded rows)         │  │ ---------------------------- │  │
│  │                             │  │ [Error Table (Bottom)]       │  │
│  └─────────────────────────────┘  └──────────────────────────────┘  │
├─────────────────────────────────────────────────────────────────────┤
│  Status: Scan + Parse complete — 32 token(s), 0 errors.             │  ← status bar
└─────────────────────────────────────────────────────────────────────┘

```

## How It Works

### Phase 1 — Lexical Analysis

The `Lexer` class reads the source string character by character using a position pointer and a simple DFA (Deterministic Finite Automaton):

1. Skip whitespace (track line/col)
2. Skip `{ ... }` comments — no token produced
3. Classify the first character:
* digit → scan NUMBER
* letter → scan identifier, check reserved word table
* `"` → scan STRING until closing `"` or error
* `:` → look ahead for `=` → ASSIGNMENTOP, else LEXERROR
* single-char symbols → direct token


4. Stamp each token with line + column

### Phase 2 — Syntax Analysis (Recursive Descent)

The `Parser` class receives the token list from the Lexer. It implements a Recursive Descent Parser — one function per non-terminal in the LL(1) grammar. Each function consumes tokens and returns an AST node.

#### Grammar Summary (LL(1) after transformations)

```text
program        → stmt-sequence
stmt-sequence  → statement stmt-sequence'
stmt-sequence' → ; statement stmt-sequence' | ε
statement      → if-stmt | repeat-stmt | assign-stmt | read-stmt | write-stmt
if-stmt        → if exp then stmt-sequence if-stmt'
if-stmt'       → else stmt-sequence end | end
repeat-stmt    → repeat stmt-sequence until exp
assign-stmt    → id := exp
read-stmt      → read id-list
write-stmt     → write exp-list
id-list        → id id-list'
id-list'       → , id id-list' | ε
exp-list       → exp-item exp-list'
exp-list'      → , exp-item exp-list' | ε
exp-item       → exp | string
exp            → simple-exp exp'
exp'           → comparison-op simple-exp exp' | ε
simple-exp     → term simple-exp'
simple-exp'    → + term simple-exp' | - term simple-exp' | ε
term           → factor term'
term'          → * factor term' | / factor term' | ε
factor         → ( exp ) | num | id

```

#### Operator Precedence (lowest to highest)

| Level | Operators | Production |
| --- | --- | --- |
| 1 | `= <` | exp |
| 2 | `+ -` | simple-exp |
| 3 | `* /` | term |
| 4 | `( atoms )` | factor |

### Error Recovery

When the parser detects a syntax error, it:

1. Records the error with type, message, line, and column
2. Calls `synchronise()` — skips tokens until a safe restart point (`;`, `end`, `else`, `until`)
3. Continues parsing — so multiple errors are found in one pass

## Error Types

| Error Type | Trigger Example | Display |
| --- | --- | --- |
| **Lexical Error** | `x @ 5`  (unknown `@`) | Red row in error table |
| **Syntax Error** | `if x then` (missing condition) | Orange row in error table |
| **Syntax Error** | `fact := ;` (missing expression) | Orange row in error table |
| **Syntax Error** | `if x < 5 then write x` (missing end) | Orange row in error table |

## Examples

### Correct Program

```pascal
{ computes factorial }
read x;
if 0 < x then
    fact := 1;
    repeat
        fact := fact * x;
        x := x - 1
    until x = 0;
    write fact
end

```

**Result:** `Syntax Analysis PASSED ✓  No errors detected.`

* Token table shows 32 tokens.
* Parse tree shows full AST starting from `[program]`. Error table automatically hides to maximize tree view.

### Program with Syntax Error

```pascal
read x
if x < 10 then
    x := x + 1

```

**Banner:** `Syntax Error at Line 2: Expected ';' but found 'if'  (2 total error(s))`

* Error table appears showing the missing `;` and missing `end`.

### Program with Lexical Error

```pascal
x := 5 @ 3

```

**Banner:** `Lexical Error at Line 1: Lexical error: unrecognized token '@' (1 total error(s))`

## Project Structure

```text
TinyLexer/
├── TinyLexer.cpp        — Entry point (main): enables visual styles, runs Form1
├── Form1.h              — Complete application in one header:
│   ├── TokenType        — enum class: 21 token categories
│   ├── Token            — struct: type, lexeme, line, col
│   ├── tokenName()      — maps TokenType → display string
│   ├── Lexer            — pure C++ scanner (DFA-based)
│   ├── ErrorKind        — enum: LEXICAL | SYNTAX
│   ├── ParseError       — struct: kind, message, line, col
│   ├── NodeKind         — enum: AST node categories
│   ├── ASTNode          — struct: tree node with children
│   ├── Parser           — pure C++ recursive descent parser
│   │   ├── parseProgram() ... parseFactor()
│   │   ├── synchronise()      — error recovery
│   │   └── populateTree()     — maps AST to UI TreeView (Updated)
│   └── Form1            — C++/CLI Windows Forms GUI
│       ├── btnScanParse_Click — runs Lexer + Parser, populates both tabs
│       ├── dgvTokens_CellFormatting — token row colors
│       ├── dgvErrors_CellFormatting — error row colors
│       └── Form1_Resize
├── AssemblyInfo.cpp     — Assembly metadata
├── Resource.h           — Resource header (VS-generated)
├── app.rc / app.ico     — Application icon
└── TinyLexer.vcxproj    — Visual Studio project file

```

## Technologies

| Technology | Role |
| --- | --- |
| **C++ (ISO Standard)** | Lexer + Parser — zero CLR dependencies |
| **C++/CLI** | Bridge: connects native C++ to .NET Windows Forms |
| **.NET Windows Forms** | GUI framework |
| **Visual Studio 2022** | IDE and build environment |
| **DataGridView** | Token table + error table |
| **RichTextBox** | Source code editor |
| **TreeView** | Interactive Abstract Syntax Tree display (Updated) |
| **SplitContainer** | Horizontal layout between AST and Errors (Updated) |
| **TabControl** | Separates Lexer output from Parser output |

## How to Run

See `SETUP_GUIDE.md` for complete Visual Studio 2022 instructions.

**Quick version:**

1. Open `TinyLexer.sln` in Visual Studio 2022
2. Set configuration: **Debug | Win32 (x86)**
3. Press **F5**

## Token Reference

| Token Type | Lexeme Example | Description |
| --- | --- | --- |
| IF_KW | `if` | Reserved keyword |
| THEN_KW | `then` | Reserved keyword |
| ELSE_KW | `else` | Reserved keyword |
| END_KW | `end` | Reserved keyword |
| REPEAT_KW | `repeat` | Reserved keyword |
| UNTIL_KW | `until` | Reserved keyword |
| READ_KW | `read` | Reserved keyword |
| WRITE_KW | `write` | Reserved keyword |
| ID | `fact`, `x` | Identifier |
| NUMBER | `0`, `42` | Integer literal |
| STRING | `"hello"` | String literal |
| ADDOP | `+` | Addition |
| SUBOP | `-` | Subtraction |
| MULOP | `*` | Multiplication |
| DIVOP | `/` | Division |
| COMPARISONOP | `=`, `<` | Comparison |
| ASSIGNMENTOP | `:=` | Assignment |
| SEMICOLON | `;` | Statement separator |
| COMMA | `,` | List separator |
| PUNCTUATION | `(`, `)` | Grouping |
| LEXERROR | `@`, `:` | Unrecognized token |

---

*Reference: Louden, K.C. (1997). Compiler Construction: Principles and Practice.*
