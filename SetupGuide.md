# Setup Guide — Visual Studio 2022
## Tiny Language Analyzer: Phase 1 + Phase 2

---

## Prerequisites

- **Windows 10 or 11**
- **Visual Studio 2022** (Community, Professional, or Enterprise)
  - Free Community edition: https://visualstudio.microsoft.com/vs/community/
- **Required workload (install once):**
  - "Desktop development with C++"
- **Required individual component (install once):**
  - "C++/CLI support for v143 build tools"

### How to verify / install the component:

1. Open **Visual Studio Installer** (search in Start Menu)
2. Click "Modify" next to your VS 2022 installation
3. Go to **"Individual Components"** tab
4. Search: `CLI`
5. Check: ✅ "C++/CLI support for v143 build tools"
6. Click **Modify** to install

---

## Option A — Open Existing Solution (Recommended)

If you already have the complete project folder with `TinyLexer.sln`:

1. Open Visual Studio 2022
2. `File → Open → Project/Solution`
3. Navigate to the project folder
4. Select `TinyLexer.sln` → click **Open**
5. In Solution Explorer, verify you see:
   - `TinyLexer.cpp`
   - `Form1.h`
   - `AssemblyInfo.cpp`
6. Go to **Step 5: Build Configuration** below

---

## Option B — Create New Project from Scratch

Use this if you need to create the project fresh.

### Step 1 — Create the Project

1. Open Visual Studio 2022
2. Click **"Create a new project"**
3. In the search box type: `CLR`
4. Select: **"CLR Empty Project (.NET Framework)"**
   - ⚠ Must say "CLR" and "C++" in the description
   - ⚠ Must say ".NET Framework" (NOT .NET 6 / .NET 8)
5. Click **Next**
6. Set:
   ```
   Project Name:   TinyLexer
   Location:       (your preferred folder)
   Framework:      .NET Framework 4.7.2
   ```
7. Click **Create**

---

### Step 2 — Add the Source Files

**Add Form1.h:**
1. Right-click **"Header Files"** in Solution Explorer
2. Select `Add → Existing Item...`
3. Navigate to and select `Form1.h`
4. Click **Add**

   *OR: `Add → New Item → Header File (.h)` → name it `Form1.h` → paste the entire code*

**Add TinyLexer.cpp:**
1. Right-click **"Source Files"** in Solution Explorer
2. Select `Add → Existing Item...`
3. Navigate to and select `TinyLexer.cpp`
4. Click **Add**

   *OR: `Add → New Item → C++ File (.cpp)` → name it `TinyLexer.cpp` → paste the code*

**Verify AssemblyInfo.cpp exists:**
- If not present, right-click Source Files → `Add → New Item → C++ File`
- Name it `AssemblyInfo.cpp`, paste the AssemblyInfo.cpp content

---

### Step 3 — Configure Project Properties

Right-click the **project name** (not the solution) in Solution Explorer  
→ **Properties**

Make ALL changes with Configuration = **"All Configurations"**  
and Platform = **"Win32"** (or "Active(Win32)")

#### A. General
```
Configuration Properties → General
  Common Language Runtime Support:
      → "Common Language Runtime Support (/clr)"
  
  Character Set:
      → "Use Unicode Character Set"
  
  .NET Target Framework Version:
      → v4.7.2
```

#### B. C/C++ → General
```
  Warning Level: Level3 (/W3)
  SDL checks: No
```

#### C. C/C++ → Precompiled Headers
```
  Precompiled Header: "Not Using Precompiled Headers"
```
*(This is critical — CLR projects without pch.h will fail otherwise)*

#### D. Linker → System
```
  SubSystem: "Windows (/SUBSYSTEM:WINDOWS)"
```

#### E. Linker → Advanced
```
  Entry Point: main
```
*(Type exactly "main" — no underscore, no brackets)*

#### F. References (must be present)
Check under Project → References:
- System
- System.Data
- System.Drawing
- System.Windows.Forms
- System.Xml

If any are missing:
- Right-click project → **Add → Reference**
- Check the missing assemblies under "Framework" tab

Click **Apply** → **OK**

---

### Step 4 — Verify the .vcxproj Settings

Open `TinyLexer.vcxproj` in a text editor and confirm  
the `Debug|Win32` ItemDefinitionGroup has:

```xml
<PropertyGroup Condition="...'Debug|Win32'..." Label="Configuration">
  <CLRSupport>true</CLRSupport>
  <CharacterSet>Unicode</CharacterSet>
</PropertyGroup>

<ItemDefinitionGroup Condition="...'Debug|Win32'...">
  <ClCompile>
    <PrecompiledHeader>NotUsing</PrecompiledHeader>
  </ClCompile>
  <Link>
    <SubSystem>Windows</SubSystem>
    <EntryPointSymbol>main</EntryPointSymbol>
  </Link>
</ItemDefinitionGroup>
```

---

### Step 5 — Build Configuration

At the top toolbar in Visual Studio, set:

```
Configuration:  Debug          (dropdown on the left)
Platform:       Win32  or  x86 (dropdown on the right)
```

⚠ **Use Win32 / x86 — NOT x64.**  
C++/CLI Windows Forms builds most reliably in 32-bit mode.

---

### Step 6 — Build the Solution

```
Build → Build Solution    (Ctrl + Shift + B)
```

Expected output in the Output window:
```
========== Build: 1 succeeded, 0 failed, 0 up-to-date, 0 skipped ==========
```

---

### Step 7 — Run the Application

```
Debug → Start Without Debugging    (Ctrl + F5)
```

The application window opens with:
- Dark-themed source code editor pre-filled with the factorial example
- Three buttons: **Scan + Parse**, **Scan Only**, **Clear**
- Two tabs: **Token Table (Lexer)** and **Syntax Analysis (Parser)**

---

## Using the Application

### Running a full analysis (Phase 1 + Phase 2):

1. Type or paste Tiny source code in the dark editor
2. Click **"Scan + Parse"**
3. If correct:
   - Banner shows green **"Syntax Analysis PASSED ✓"**
   - Token Table tab: all tokens color-coded
   - Parser tab (left): indented AST tree
   - Parser tab (right): empty error table
4. If errors exist:
   - Banner shows red **"Syntax Error at Line X: ..."**
   - Parser tab automatically opens
   - Error table lists every error with type, line, column, message

### Running lexer only (Phase 1):

1. Click **"Scan Only"**
2. Token table fills; no parsing is performed

### Clearing:

1. Click **"Clear"** to reset everything

---

## Troubleshooting

### Error: "Cannot open include file: 'msclr/marshal_cppstd.h'"
**Fix:** The project is not configured as CLR.  
Go to Properties → General → Common Language Runtime Support → set to `/clr`

### Error: "LNK2019: unresolved external symbol _main"
**Fix:** Go to Properties → Linker → Advanced → Entry Point  
Type: `main` (exactly, no underscore)

### Error: "pch.h: No such file"
**Fix:** Properties → C/C++ → Precompiled Headers  
Set "Precompiled Header" to **"Not Using Precompiled Headers"**

### Application shows a console window behind the GUI
**Fix:** Properties → Linker → System → SubSystem  
Set to: **"Windows (/SUBSYSTEM:WINDOWS)"**

### Build succeeds but app crashes immediately on launch
**Fix:** Make sure you are using **Win32 platform**, not x64.  
Change the platform dropdown at the top to "x86" or "Win32".

### Error: "error C2664: cannot convert 'const char*' to 'System::String^'"
**Fix:** Ensure `#include <msclr/marshal_cppstd.h>` is at the top of Form1.h  
and CLR support is enabled in project properties.

### IntelliSense shows red squiggles but build succeeds
This is normal for C++/CLI projects — IntelliSense and the CLR compiler  
use different parsers. Trust the build output, not IntelliSense.

---

## File Summary

| File               | Purpose                                          |
|--------------------|--------------------------------------------------|
| `Form1.h`          | All application logic + GUI (Lexer + Parser)     |
| `TinyLexer.cpp`    | Entry point — calls Application::Run(Form1)      |
| `AssemblyInfo.cpp` | .NET assembly metadata (VS-generated)            |
| `Resource.h`       | Resource IDs (VS-generated, can be empty)        |
| `TinyLexer.vcxproj`| Visual Studio project configuration              |
| `TinyLexer.sln`    | Visual Studio solution file                      |
| `.gitignore`       | Git ignore rules for VS build artifacts          |
| `README.md`        | Project documentation                            |
| `Phase2_Theory.md` | CFG, First/Follow sets, LL(1) parse table        |
| `SETUP_GUIDE.md`   | This file                                        |

---

*Tested on: Visual Studio 2022 v17.x, Windows 10/11, .NET Framework 4.7.2, Win32 platform.*
