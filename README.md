# Tiny Language Lexical Analyzer

> A GUI-based lexical scanner for the Tiny programming language — built with C++/CLI and Windows Forms.

---

## Overview

The **Tiny Language Lexical Analyzer** is the first stage of a compiler pipeline, implemented as a fully interactive desktop application. It accepts source code written in the **Tiny language** — a compact, educational programming language commonly taught in compiler design courses — and produces a structured, color-coded token table in real time.

Rather than a command-line tool, this project delivers the scanning experience through a clean Windows Forms GUI: type or paste your Tiny source code, click **Scan / Tokenize**, and instantly see every token classified by type, along with its exact position (line and column) in the source. Lexical errors are flagged in red so they stand out immediately.

This project was built as a practical demonstration of how a real compiler's front-end works — not as a toy exercise, but as a properly engineered component with a clean separation between the lexer logic (pure standard C++) and the presentation layer (C++/CLI Windows Forms).

---

## Features

- **Full Tiny language token support** — keywords, identifiers, numbers, string literals, arithmetic and comparison operators, assignment, punctuation, and end-of-file
- **Accurate error detection** — unterminated strings, lone `:` without `=`, and unknown characters are reported as `LEXERROR` tokens
- **Precise source location tracking** — every token records its line number and column, making it straightforward to extend into a parser or error-reporter
- **Color-coded output table** — each token category is rendered in a distinct color for immediate visual comprehension:
  - Keywords → blue
  - Identifiers → green
  - Numbers → orange
  - String literals → purple
  - Operators → yellow
  - Errors → red
- **Comment stripping** — `{ ... }` block comments are silently consumed; they never appear in the token stream
- **Responsive layout** — the GUI resizes gracefully, keeping all controls stretched to the window width
- **Status bar feedback** — after every scan, a status message reports the total token count and the number of errors detected
- **Pre-loaded example** — the editor opens with a working Tiny factorial program so new users can run their first scan immediately

---

## Screenshot

> *Place your application screenshot here.*

```
┌──────────────────────────────────────────────────────┐
│  Tiny Language _ Lexical Analyzer                    │
├──────────────────────────────────────────────────────┤
│  Source Code Editor (dark theme, Consolas font)      │
│  { Sample Tiny program — computes factorial }        │
│  read x;                                             │
│  if 0 < x then                                       │
│    fact := 1;                                        │
│    ...                                               │
├──────────────────────────────────────────────────────┤
│  [ Scan / Tokenize ]  [ Clear ]                      │
├──────────────────────────────────────────────────────┤
│  #  │ Token Type   │ Lexeme │ Line │ Column          │
│  1  │ READ_KW      │ read   │  2   │  1              │
│  2  │ ID           │ x      │  2   │  6              │
│  3  │ SEMICOLON    │ ;      │  2   │  7              │
│  ...                                                 │
│  Status: Scan complete — 28 token(s) found.          │
└──────────────────────────────────────────────────────┘
```

---

## How Lexical Analysis Works

A **lexer** (also called a scanner or tokenizer) is the very first phase of a compiler. Its job is to read raw source code — which is just a sequence of characters — and group those characters into meaningful units called **tokens**.

Think of it like reading a sentence in English. Before you can understand the meaning ("parsing"), you first need to recognise individual words. A lexer does exactly the same thing for programming languages.

This lexer works as follows:

1. It reads the source string one character at a time using a position pointer.
2. Whitespace and `{ ... }` comments are silently discarded — they carry no semantic meaning.
3. Each remaining character triggers a classification rule:
   - A digit starts a **NUMBER** token; the lexer consumes consecutive digits.
   - A letter starts a keyword or **IDENTIFIER** scan; the result is checked against the reserved word table.
   - A `"` starts a **STRING_LIT** scan; it reads until the closing `"` or a newline (which triggers an error).
   - Single-character symbols (`+`, `-`, `*`, `/`, `=`, `<`, `;`, `,`, `(`, `)`) map directly to their token types.
   - `:` looks ahead for `=` to produce `ASSIGNMENTOP` (`:=`); otherwise it is a `LEXERROR`.
   - Anything else is a `LEXERROR`.
4. Every token is stamped with its **line** and **column** before being appended to the token list.
5. Scanning ends when the end of the source string is reached, which produces an `ENDOFFILE` token.

---

## Example: Input and Output

### Input — Tiny factorial program

```tiny
{ Sample Tiny program — computes factorial }
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

> **Note:** The `{ ... }` block on line 1 is a comment and produces no tokens.

### Output — Token Table
---------------------------------------------
| # | Token Type   | Lexeme | Line | Column |
|---|--------------|--------|------|--------|
| 1 | READ_KW      | read   | 2    | 1      |
| 2 | ID           | x      | 2    | 6      |
| 3 | SEMICOLON    | ;      | 2    | 7      |
| 4 | IF_KW        | if     | 3    | 1      |
| 5 | NUMBER       | 0      | 3    | 4      |
| 6 | COMPARISONOP | <      | 3    | 6      |
| 7 | ID           | x      | 3    | 8      |
| 8 | THEN_KW      | then   | 3    | 10     |
| 9 | ID           | fact   | 4    | 5      |
| 10 | ASSIGNMENTOP | :=    | 4    | 10     |
| 11 | NUMBER       | 1      | 4    | 13    |
| 12 | SEMICOLON    | ;      | 4    | 14    |
| 13 | REPEAT_KW    | repeat | 5    | 5     |
| 14 | ID           | fact   | 6    | 9     |
| 15 | ASSIGNMENTOP | :=     | 6    | 14    |
| 16 | ID           | fact   | 6    | 17    |
| 17 | MULOP        | *      | 6    | 22    |
| 18 | ID           | x      | 6    | 24    |
| 19 | SEMICOLON    | ;      | 6    | 25    |
| 20 | ID           | x      | 7    | 9     |
| 21 | ASSIGNMENTOP | :=     | 7    | 11    |
| 22 | ID           | x      | 7    | 14    |
| 23 | SUBOP        | -      | 7    | 16    |
| 24 | NUMBER       | 1      | 7    | 18    |
| 25 | UNTIL_KW     | until  | 8    | 5     |
| 26 | ID           | x      | 8    | 11    |
| 27 | COMPARISONOP | =      | 8    | 13    |
| 28 | NUMBER       | 0      | 8    | 15    |
| 29 | SEMICOLON    | ;      | 8    | 16    |
| 30 | WRITE_KW     | write  | 9    | 5     |
| 31 | ID           | fact   | 9    | 11    |
| 32 | END_KW       | end    | 10   | 1     |
---------------------------------------------
**Status bar:** `Scan complete — 32 token(s) found successfully. No errors detected.`

### Error Example

Input with a lexical error:

```tiny
x : 5
```

| # | Token Type | Lexeme | Line | Column |
|---|------------|--------|------|--------|
| 1 | ID         | x      | 1    | 1      |
| 2 | ERROR      | :      | 1    | 3      |
| 3 | NUMBER     | 5      | 1    | 5      |

**Status bar:** `Scan complete — 3 token(s) found, 1 error(s) detected. (Errors shown in red)`

---

## Technologies Used

| Technology | Role |
|---|---|
| **C++ (ISO Standard)** | Core lexer logic — zero CLR dependencies inside the `Lexer` class |
| **C++/CLI** | Bridge layer connecting native C++ to the .NET Windows Forms runtime |
| **.NET Windows Forms** | GUI framework — form, controls, and event handling |
| **Visual Studio 2022** | IDE and build environment |
| **DataGridView** | Structured, color-coded token output table |
| **RichTextBox** | Dark-themed source code editor |

The architecture deliberately keeps the `Lexer` class as a pure standard C++ class. It has no managed types, no CLR headers, and no Windows Forms dependencies. This means the lexer could be extracted and reused in a purely native C++ project, a web server, or any other context without modification. The C++/CLI `Form1` class acts as a thin presentation wrapper only.

---

## Project Structure

```
TinyLexer/
├── TinyLexer.cpp       # Application entry point — enables visual styles, launches Form1
└── Form1.h             # All logic and UI in a single header:
                        #   ├── TokenType  (enum class — 21 token categories)
                        #   ├── Token      (struct — type, lexeme, line, col)
                        #   ├── tokenName  (maps TokenType → display string)
                        #   ├── Lexer      (pure C++ scanner class)
                        #   │     ├── peek / advance / match  (character primitives)
                        #   │     ├── skipWhitespace / skipComment
                        #   │     ├── nextToken               (core dispatch logic)
                        #   │     └── tokenize                (produces full token list)
                        #   └── Form1      (C++/CLI Windows Form)
                        #         ├── InitializeComponent     (builds the entire UI)
                        #         ├── btnScan_Click           (invokes lexer, populates grid)
                        #         ├── btnClear_Click          (resets editor and table)
                        #         ├── dgv_CellFormatting      (color-codes rows by type)
                        #         └── Form1_Resize            (keeps controls responsive)
```

---

## How to Run

### Prerequisites

- Windows 10 or later
- Visual Studio 2022 (any edition — Community is free)
- Workload installed: **Desktop development with C++**
- Component installed: **C++/CLI support for v143 build tools**

### Steps

1. **Clone or download** this repository.

2. **Open the solution** in Visual Studio 2022:
   ```
   File → Open → Project/Solution → TinyLexer.sln
   ```

3. **Set the build configuration** to `Debug` or `Release` and the platform to `**x86**`.

4. **Build the project:**
   ```
   Build → Build Solution   (Ctrl + Shift + B)
   ```

5. **Run the application:**
   ```
   Debug → Start Without Debugging   (Ctrl + F5)
   ```

6. The application opens with a pre-loaded factorial example. Click **Scan / Tokenize** to run your first analysis immediately, or clear the editor and type your own Tiny source code.

> **Important:** This project requires a Windows environment. It cannot be compiled or run on Linux or macOS, as it targets the Windows Forms / .NET runtime via C++/CLI.

---

## Token Type Reference

| Token Type    | Example Lexeme | Description                        |
|---------------|----------------|------------------------------------|
| `IF_KW`       | `if`           | Reserved keyword                   |
| `THEN_KW`     | `then`         | Reserved keyword                   |
| `ELSE_KW`     | `else`         | Reserved keyword                   |
| `END_KW`      | `end`          | Reserved keyword                   |
| `REPEAT_KW`   | `repeat`       | Reserved keyword                   |
| `UNTIL_KW`    | `until`        | Reserved keyword                   |
| `READ_KW`     | `read`         | Reserved keyword                   |
| `WRITE_KW`    | `write`        | Reserved keyword                   |
| `ID`          | `fact`, `x`    | User-defined identifier            |
| `NUMBER`      | `0`, `42`      | Integer literal                    |
| `STRING_LIT`  | `"hello"`      | Double-quoted string literal       |
| `ADDOP`       | `+`            | Addition operator                  |
| `SUBOP`       | `-`            | Subtraction operator               |
| `MULOP`       | `*`            | Multiplication operator            |
| `DIVOP`       | `/`            | Division operator                  |
| `COMPARISONOP`| `=`, `<`       | Relational operator                |
| `ASSIGNMENTOP`| `:=`           | Assignment operator                |
| `SEMICOLON`   | `;`            | Statement terminator               |
| `COMMA`       | `,`            | Separator                          |
| `PUNCTUATION` | `(`, `)`       | Grouping                           |
| `ENDOFFILE`   | `EOF`          | End of source (not shown in table) |
| `LEXERROR`    | `:`, `@`, ...  | Unrecognized or malformed token    |

---

## Future Improvements

This lexer is designed to serve as the foundation of a complete compiler front-end. Natural next steps include:

- **Recursive-descent parser** — consume the token stream produced by this lexer and build a concrete syntax tree (CST) for Tiny programs
- **Abstract Syntax Tree (AST) visualizer** — render the tree structure graphically inside the same GUI
- **Syntax highlighting in the editor** — colorize the source code input in real time as the user types, using the same token categories
- **File I/O** — open `.tiny` source files from disk and export the token table as CSV or JSON
- **Semantic analysis phase** — add a symbol table and type-checker on top of the parser
- **Extended language support** — generalize the lexer to handle other small educational languages (e.g., PL/0, µ-Pascal)
- **Unit test suite** — formalize the lexer's behaviour with a set of known-good input/output pairs using a C++ testing framework

---

## Author

Built by a Computer Science student as a compiler design course project.

Contributions, suggestions, and issue reports are welcome — feel free to open a pull request or file an issue.

---

*Tiny language specification reference: Kenneth C. Louden, "Compiler Construction: Principles and Practice" (1997).*

*Note: The theoretical models (Unified Regular Expression, NFA, and DFA diagrams) for the Tiny language 
  are included as softcopy PDF/Image files in the Docs folder as per the project requirements.*
