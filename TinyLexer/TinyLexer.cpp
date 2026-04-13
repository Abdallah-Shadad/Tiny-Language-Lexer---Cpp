// TinyLexer.cpp — Entry point
// C++/CLI Windows Forms Application
// Tiny Language Lexical Analyzer
#include "Form1.h"
using namespace TinyLexer;

[System::STAThreadAttribute]
int main() {
	Application::EnableVisualStyles();
	Application::SetCompatibleTextRenderingDefault(false);
	Application::Run(gcnew Form1());
	return 0;
}
