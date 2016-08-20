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
// Note: This grammar includes whitespace handling, so there is no tokenizer.
//
// Parsing Grammars - PG
//
//   ()              Empty string, always succeeds.
//   a               Terminal, consume and succeed.
//   A               Non-terminal, recurse and succeed.
//   e1,e2, ...en    Sequence, e1 and e2 ...
//   e1/e2/.../en    Choice, e1 or e2 ...
//   e*              Greedy repetition, always succeeds.
//   e+              Greedy positive repetition.
//   e?              Optional.
//   &(e)            Positive lookahead, succeed in presence of e but do not consume e.
//   !(e)            Negative lookahead, succeed in absence of e but do not consume e.
//
// Grammar Grammar:
//

/*
TODO  Rename 'Definition' to 'Rule'
*/

//   Grammar        <-- Spacing Definition+ EOF
//   Definition     <-- Identifier LEFTARROW Sequence+ DEFTERMINATOR
//   Sequence       <-- Alternative (SLASH Alternative)*
//   Alternative    <-- AND UnaryItem / NOT UnaryItem / UnaryItem
//   UnaryItem      <-- PrimaryItem QUESTION / PrimaryItem STAR / PrimaryItem PLUS / PrimaryItem
//   PrimaryItem    <-- Identifier / CharLiteral / StringLiteral / OPEN Sequence? CLOSE
//   Identifier     <-- IdentStart IdentCont* Spacing
//   IdentStart     <-- Alpha / '_'
//   IdentCont      <-- IdentStart / Digit / '''
//   CharLiteral    <-- ''' (!(''') QuotedChar) ''' Spacing
//   StringLiteral  <-- '"' (!('"') QuotedChar)* '"' Spacing
//   QuotedChar     <-- '\n' / '\r' / '\t' / '\\' / '\'' / '\"' / !('\') Char
//   Spacing        <-- (SpaceChar / LineComment)*
//   SpaceChar      <-- ' ' / TAB / CR / LF
//   LineComment    <-- '#' (!(LineTerminator) Char)* LineTerminator
//   LineTerminator <-- CR LF / CR / LF
//   CR             <-- '\r'
//   LF             <-- '\n'
//   TAB            <-- '\t'
//   EOF            <-- !(Char)
//   AND            <-- '&' Spacing
//   NOT            <-- '!' Spacing
//   QUESTION       <-- '?' Spacing
//   STAR           <-- '*' Spacing
//   PLUS           <-- '+' Spacing
//   LEFTARROW      <-- '<--' Spacing
//   OPEN           <-- '(' Spacing
//   CLOSE          <-- ')' Spacing
//   SLASH          <-- '/' Spacing
//
//   Built-ins:
//     Alpha
//     Digit
//     Char
//
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
        std::cout << "# Grammar::isGrammar " << pig.cursor () << " [" << pig.peek (16) << "]\n";
        return true;
      }
    }
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Definition <-- Identifier LEFTARROW Sequence+ DEFTERMINATOR
bool Grammar::isDefinition (Pig& pig)
{
  auto checkpoint = pig.cursor ();
  if (isIdentifier (pig)        &&
      isLiteral    (pig, "<--") &&
      isSequence   (pig))
  {
    while (isSequence (pig))
    {
    }

    if (isLiteral (pig, ";"))
    {
      std::cout << "# Grammar::isDefinition " << pig.cursor () << " [" << pig.peek (16) << "]\n";
      return true;
    }
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Sequence <-- Alternative (SLASH Alternative)*
bool Grammar::isSequence (Pig& pig)
{
  if (isAlternative (pig))
  {
    while (isLiteral (pig, "/") &&
           isAlternative (pig))
    {
    }

    std::cout << "# Grammar::isSequence " << pig.cursor () << " [" << pig.peek (16) << "]\n";
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Alternative <-- AND UnaryItem / NOT UnaryItem / UnaryItem
bool Grammar::isAlternative (Pig& pig)
{
  auto checkpoint = pig.cursor ();
  if (isLiteral (pig, "&") &&
      isUnaryItem (pig))
  {
    std::cout << "# Grammar::isAlternative " << pig.cursor () << " [" << pig.peek (16) << "]\n";
    return true;
  }

  pig.restoreTo (checkpoint);
  if (isLiteral (pig, "!") &&
      isUnaryItem (pig))
  {
    std::cout << "# Grammar::isAlternative " << pig.cursor () << " [" << pig.peek (16) << "]\n";
    return true;
  }

  pig.restoreTo (checkpoint);
  if (isUnaryItem (pig))
  {
    std::cout << "# Grammar::isAlternative " << pig.cursor () << " [" << pig.peek (16) << "]\n";
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
    if (isLiteral (pig, "?") ||
        isLiteral (pig, "*") ||
        isLiteral (pig, "+"))
    {
    }

    std::cout << "# Grammar::isUnaryItem " << pig.cursor () << " [" << pig.peek (16) << "]\n";
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// PrimaryItem  <-- Identifier / CharLiteral / StringLiteral / OPEN Sequence? CLOSE
bool Grammar::isPrimaryItem (Pig& pig)
{
  if (isIdentifier    (pig) ||
      isCharLiteral   (pig) ||
      isStringLiteral (pig))
  {
    std::cout << "# Grammar::isPrimaryItem " << pig.cursor () << " [" << pig.peek (16) << "]\n";
    return true;
  }

  auto checkpoint = pig.cursor ();
  if (isLiteral (pig, "("))
  {
    isSequence (pig);
    if (isLiteral (pig, ")"))
    {
      std::cout << "# Grammar::isPrimaryItem " << pig.cursor () << " [" << pig.peek (16) << "]\n";
      return true;
    }
  }

  pig.restoreTo (checkpoint);
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
    std::cout << "# Grammar::isIdentifier " << pig.cursor () << " [" << pig.peek (16) << "]\n";
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
    std::cout << "# Grammar::isIdentStart " << pig.cursor () << " [" << pig.peek (16) << "]\n";
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
    std::cout << "# Grammar::isIdentCont " << pig.cursor () << " [" << pig.peek (16) << "]\n";
    return true;
  }

  if (unicodeLatinDigit (pig.peek ()))
  {
    pig.skipN (1);
    std::cout << "# Grammar::isIdentCont " << pig.cursor () << " [" << pig.peek (16) << "]\n";
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
      value.length () == 1        &&
      isSpacing (pig))
  {
    std::cout << "# Grammar::isCharLiteral " << pig.cursor () << " [" << pig.peek (16) << "]\n";
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
  if (pig.getQuoted ('"', value) &&
      isSpacing (pig))
  {
    std::cout << "# Grammar::isStringLiteral " << pig.cursor () << " [" << pig.peek (16) << "]\n";
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

    std::cout << "# Grammar::isLineComment " << pig.cursor () << " [" << pig.peek (16) << "]\n";
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
    std::cout << "# Grammar::isLineTerminator " << pig.cursor () << " [" << pig.peek (16) << "]\n";
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Grammar::isLiteral (
  Pig& pig,
  const std::string& thing)
{
  if (pig.skipLiteral (thing) &&
      isSpacing (pig))
  {
    std::cout << "# Grammar::isLiteral('" << thing << "') " << pig.cursor () << " [" << pig.peek (16) << "]\n";
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
