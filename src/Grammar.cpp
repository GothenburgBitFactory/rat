////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2015 - 2016, Paul Beckingham, Federico Hernandez.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// http://www.opensource.org/licenses/mit-license.php
//
////////////////////////////////////////////////////////////////////////////////

#include <cmake.h>
#include <Grammar.h>
#include <Lexer.h>
#include <shared.h>
#include <unicode.h>
#include <format.h>
#include <iostream>
#include <sstream>

////////////////////////////////////////////////////////////////////////////////
// Load and parse PG.
void Grammar::initialize (const std::string& input)
{
  std::string rule_name = "";

  // This is a state machine.  Read each line.
  for (auto& line : split (input, '\n'))
  {
    // Skip whole-line comments.
    if (line[0] == '#')
      continue;

    // Eliminate inline comments.
    std::string::size_type hash = line.find ('#');
    if (hash != std::string::npos)
      line.resize (hash);

////////////////////////////////////////////////////////////////////////////////
// Definition <-- Identifier LEFTARROW Rule
bool Grammar::isDefinition (Pig& pig)
{
  std::cout << "Grammar::isDefinition " << pig.dump () << "\n";
  auto checkpoint = pig.cursor ();
  if (isIdentifier (pig) &&
      isLeftArrow  (pig) &&
      isRule       (pig))
  {
    std::cout << "# Grammar::isDefinition\n";
    return true;
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Rule <-- Sequence+
bool Grammar::isRule (Pig& pig)
{
  // TODO Implement.
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Identifier <-- IdentStart IdentCont* Spacing
bool Grammar::isIdentifier (Pig& pig)
{
  if (isIdentStart (pig))
  {
    while (isIdentCont (pig))
    {
    }

    isSpacing (pig);
    std::cout << "# Grammar::isIdentifier\n";
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// IdentStart <-- Alpha / '_'
bool Grammar::isIdentStart (Pig& pig)
{
  if (unicodeLatinAlpha (pig.peek ()) ||
      pig.peek () == '_')
  {
    pig.skipN (1);
    std::cout << "# Grammar::isIdentStart\n";
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// IdentCont <-- IdentStart / Digit
// What is this?   IdentCont <-- IdentStart / Digit / '''
bool Grammar::isIdentCont (Pig& pig)
{
  if (isIdentStart (pig))
    return true;

  if (unicodeLatinDigit (pig.peek ()))
  {
    pig.skipN (1);
    std::cout << "# Grammar::isIdentCont\n";
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Spacing <-- (SpaceChar / LineComment)*
bool Grammar::isSpacing (Pig& pig)
{
  while (isSpaceChar   (pig) ||
         isLineComment (pig))
  {
  }

  // Always true because of '*'.
  return true;
}

////////////////////////////////////////////////////////////////////////////////
// SpaceChar <-- ' ' / TAB / CR / LF
bool Grammar::isSpaceChar (Pig& pig)
{
  return pig.skip (' ')  ||
         pig.skip ('\t') ||
         pig.skip ('\r') ||
         pig.skip ('\f') ||
         pig.skip ('\n');
}

////////////////////////////////////////////////////////////////////////////////
// LineComment <-- '#' (!(LineTerminator) Char)* LineTerminator
bool Grammar::isLineComment (Pig& pig)
{
  if (pig.skip ('#'))
  {
    while (! isEOF (pig) &&
           ! isLineTerminator (pig))
    {
      pig.skipN (1);
    }

    std::cout << "# Grammar::isLineComment\n";
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// LineTerminator <-- CR LF / CR / LF
bool Grammar::isLineTerminator (Pig& pig)
{
  if (pig.skip ('\n') ||
      pig.skip ('\r') ||
      pig.skip ('\f'))
  {
    pig.skip ('\f');
    std::cout << "# Grammar::isLineTerminator\n";
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// LEFTARROW <-- '<--' Spacing
bool Grammar::isLeftArrow (Pig& pig)
{
  if (pig.skipLiteral ("<--") &&
      isSpacing (pig))
  {
    std::cout << "# Grammar::isLeftArrow\n";
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// EOF <-- !(Char)
bool Grammar::isEOF (Pig& pig)
{
  return pig.eos ();
}

////////////////////////////////////////////////////////////////////////////////
void Grammar::debug (bool value)
{
  _debug = value;
}

////////////////////////////////////////////////////////////////////////////////
std::string Grammar::dump () const
{
  std::stringstream out;
  out << "Grammar\n";

  // TODO Dump state here.

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
