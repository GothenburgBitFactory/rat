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
// Tokenizer/Lexer-free.
void Grammar::initialize (const std::string& input)
{
  Pig pig (input);
  if (! isGrammar (pig))
    throw std::string ("Syntax error in grammar.");
}

////////////////////////////////////////////////////////////////////////////////
// Grammar <-- Spacing Definition+ EOF
bool Grammar::isGrammar (Pig& pig)
{
  auto checkpoint = pig.cursor ();
  if (isSpacing (pig))
  {
    if (isDefinition (pig))
    {
      while (isDefinition (pig))
      {
      }

      if (isEOF (pig))
      {
        std::cout << "# Grammar::isGrammar " << pig.cursor () << " (" << pig.peek (16) << ")\n";
        return true;
      }
    }
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Definition <-- Identifier LEFTARROW Rule
bool Grammar::isDefinition (Pig& pig)
{
  auto checkpoint = pig.cursor ();
  if (isIdentifier (pig) &&
      isLeftArrow  (pig) &&
      isRule       (pig))
  {
    std::cout << "# Grammar::isDefinition " << pig.cursor () << " (" << pig.peek (16) << ")\n";
    return true;
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Rule <-- Sequence+
bool Grammar::isRule (Pig& pig)
{
  if (isSequence (pig))
  {
    while (isSequence (pig))
    {
    }

    std::cout << "# Grammar::isRule " << pig.cursor () << " (" << pig.peek (16) << ")\n";
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Sequence <-- Alternative (SLASH Alternative)*
bool Grammar::isSequence (Pig& pig)
{
  if (isAlternative (pig))
  {
    while (isSlash (pig) &&
           isAlternative (pig))
    {
    }

    std::cout << "# Grammar::isSequence " << pig.cursor () << " (" << pig.peek (16) << ")\n";
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Alternative <-- AND UnaryItem / NOT UnaryItem / UnaryItem
bool Grammar::isAlternative (Pig& pig)
{
  auto checkpoint = pig.cursor ();
  if (isAnd (pig) &&
      isUnaryItem (pig))
  {
    std::cout << "# Grammar::isAlternative " << pig.cursor () << " (" << pig.peek (16) << ")\n";
    return true;
  }

  pig.restoreTo (checkpoint);
  if (isNot (pig) &&
      isUnaryItem (pig))
  {
    std::cout << "# Grammar::isAlternative " << pig.cursor () << " (" << pig.peek (16) << ")\n";
    return true;
  }

  pig.restoreTo (checkpoint);
  if (isUnaryItem (pig))
  {
    std::cout << "# Grammar::isAlternative " << pig.cursor () << " (" << pig.peek (16) << ")\n";
    return true;
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// UnaryItem <-- PrimaryItem QUESTION / PrimaryItem STAR / PrimaryItem PLUS / PrimaryItem
bool Grammar::isUnaryItem (Pig& pig)
{
  if (isPrimaryItem (pig))
  {
    if (isQuestion (pig) ||
        isStar     (pig) ||
        isPlus     (pig))
    {
    }

    std::cout << "# Grammar::isUnaryItem " << pig.cursor () << " (" << pig.peek (16) << ")\n";
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// PrimaryItem  <-- Identifier / CharLiteral / StringLiteral / OPEN Sequence CLOSE
bool Grammar::isPrimaryItem (Pig& pig)
{
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
    std::cout << "# Grammar::isIdentifier " << pig.cursor () << " (" << pig.peek (16) << ")\n";
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
    std::cout << "# Grammar::isIdentStart " << pig.cursor () << " (" << pig.peek (16) << ")\n";
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
  {
    std::cout << "# Grammar::isIdentCont " << pig.cursor () << " (" << pig.peek (16) << ")\n";
    return true;
  }

  if (unicodeLatinDigit (pig.peek ()))
  {
    pig.skipN (1);
    std::cout << "# Grammar::isIdentCont " << pig.cursor () << " (" << pig.peek (16) << ")\n";
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// CharLiteral <-- ''' (!(''') QuotedChar) ''' Spacing
bool Grammar::isCharLiteral (Pig& pig)
{
  auto checkpoint = pig.cursor ();
  std::string value;
  if (pig.getQuoted ('\'', value) &&
      value.length () == 1)
  {
    std::cout << "# Grammar::isCharLiteral " << pig.cursor () << " (" << pig.peek (16) << ")\n";
    return true;
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// StringLiteral <-- '"' (!('"') QuotedChar)* '"' Spacing
bool Grammar::isStringLiteral (Pig& pig)
{
  std::string value;
  if (pig.getQuoted ('"', value))
  {
    std::cout << "# Grammar::isStringLiteral " << pig.cursor () << " (" << pig.peek (16) << ")\n";
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

    std::cout << "# Grammar::isLineComment " << pig.cursor () << " (" << pig.peek (16) << ")\n";
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
    std::cout << "# Grammar::isLineTerminator " << pig.cursor () << " (" << pig.peek (16) << ")\n";
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// OPEN <-- '(' Spacing
bool Grammar::isOpen (Pig& pig)
{
  if (pig.skip ('(') &&
      isSpacing (pig))
  {
    std::cout << "# Grammar::isOpen " << pig.cursor () << " (" << pig.peek (16) << ")\n";
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// SLASH <-- '/' Spacing
bool Grammar::isSlash (Pig& pig)
{
  if (pig.skip ('/') &&
      isSpacing (pig))
  {
    std::cout << "# Grammar::isSlash " << pig.cursor () << " (" << pig.peek (16) << ")\n";
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// AND <-- '&' Spacing
bool Grammar::isAnd (Pig& pig)
{
  if (pig.skip ('&') &&
      isSpacing (pig))
  {
    std::cout << "# Grammar::isAnd " << pig.cursor () << " (" << pig.peek (16) << ")\n";
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// NOT <-- '!' Spacing
bool Grammar::isNot (Pig& pig)
{
  if (pig.skip ('!') &&
      isSpacing (pig))
  {
    std::cout << "# Grammar::isNot " << pig.cursor () << " (" << pig.peek (16) << ")\n";
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// QUESTION <-- '!' Spacing
bool Grammar::isQuestion (Pig& pig)
{
  if (pig.skip ('?') &&
      isSpacing (pig))
  {
    std::cout << "# Grammar::isQuestion " << pig.cursor () << " (" << pig.peek (16) << ")\n";
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// STAR <-- '!' Spacing
bool Grammar::isStar (Pig& pig)
{
  if (pig.skip ('*') &&
      isSpacing (pig))
  {
    std::cout << "# Grammar::isStar " << pig.cursor () << " (" << pig.peek (16) << ")\n";
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// PLUS <-- '!' Spacing
bool Grammar::isPlus (Pig& pig)
{
  if (pig.skip ('+') &&
      isSpacing (pig))
  {
    std::cout << "# Grammar::isPlus " << pig.cursor () << " (" << pig.peek (16) << ")\n";
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
    std::cout << "# Grammar::isLeftArrow " << pig.cursor () << " (" << pig.peek (16) << ")\n";
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
  std::cout << "Grammar debug mode.\n";
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
