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
#include <cassert>

////////////////////////////////////////////////////////////////////////////////
// Load and parse PG.
// Note: This grammar includes whitespace handling, so there is no tokenizer.
//
// Parsing Grammars - PG
//
//   ()              Empty string, always succeeds.
//   a               Terminal, consume and succeed.
//   A               Non-terminal, recurse and succeed.
//   e1 e2 ... en    Sequence, e1 and e2 ...
//   e1/e2/... en    Choice, e1 or e2 ...
//   e*              Greedy repetition, always succeeds.
//   e+              Greedy positive repetition.
//   e?              Optional.
//   &(e)            Positive lookahead, succeed in presence of e but do not consume e.
//   !(e)            Negative lookahead, succeed in absence of e but do not consume e.
//
// Grammar Grammar:
//
//   Grammar        <-- Spacing Rule+ EOF
//   Rule           <-- Identifier LEFTARROW Sequence+ DEFTERMINATOR
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
  _rules->_name = "Grammar";
  if (! isGrammar (pig, _rules))
    throw std::string ("Syntax error in grammar.");

  std::cout << "Grammar loaded.\n";
}

////////////////////////////////////////////////////////////////////////////////
std::shared_ptr <Tree> Grammar::syntax () const
{
  assert (_rules->_branches.size ());
  return _rules;
}

////////////////////////////////////////////////////////////////////////////////
// Grammar <-- Spacing Rule+ EOF
bool Grammar::isGrammar (Pig& pig, std::shared_ptr <Tree> parseTree)
{
  auto checkpoint = pig.cursor ();
  if (isSpacing (pig))
  {
    std::shared_ptr <Tree> rule = std::make_shared <Tree> ();
    rule->_name = "Rule";
    if (isRule (pig, rule))
    {
      parseTree->addBranch (rule);

      rule = std::make_shared <Tree> ();
      rule->_name = "Rule";
      while (isRule (pig, rule))
      {
        parseTree->addBranch (rule);
        rule = std::make_shared <Tree> ();
        rule->_name = "Rule";
      }

      if (isEOF (pig))
        return true;
    }
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Rule <-- Identifier LEFTARROW Sequence+ DEFTERMINATOR
bool Grammar::isRule (Pig& pig, std::shared_ptr <Tree> parseTree)
{
  auto checkpoint = pig.cursor ();

  std::shared_ptr <Tree> name = std::make_shared <Tree> ();
  name->_name = "Identifier";
  if (isIdentifier (pig, name) &&
      isLiteral    (pig, "<--"))
  {
    std::vector <std::shared_ptr <Tree>> sequences;
    std::shared_ptr <Tree> sequence = std::make_shared <Tree> ();
    sequence->_name = "Sequence";
    if (isSequence (pig, sequence))
    {
      sequences.push_back (sequence);

      sequence = std::make_shared <Tree> ();
      sequence->_name = "Sequence";
      while (isSequence (pig, sequence))
      {
        sequences.push_back (sequence);
        sequence = std::make_shared <Tree> ();
        sequence->_name = "Sequence";
      }

      if (isLiteral (pig, ";"))
      {
        parseTree->addBranch (name);
        for (auto s : sequences)
          parseTree->addBranch (s);

        return true;
      }
    }
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Sequence <-- Alternative (SLASH Alternative)*
bool Grammar::isSequence (Pig& pig, std::shared_ptr <Tree> parseTree)
{
  std::shared_ptr <Tree> alternative = std::make_shared <Tree> ();
  alternative->_name = "Alternative";
  if (isAlternative (pig, alternative))
  {
    parseTree->addBranch (alternative);

    alternative = std::make_shared <Tree> ();
    alternative->_name = "Alternative";
    while (isLiteral (pig, "/") &&
           isAlternative (pig, alternative))
    {
      parseTree->addBranch (alternative);
      alternative = std::make_shared <Tree> ();
      alternative->_name = "Alternative";
    }

    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Alternative <-- AND UnaryItem / NOT UnaryItem / UnaryItem
bool Grammar::isAlternative (Pig& pig, std::shared_ptr <Tree> parseTree)
{
  auto checkpoint = pig.cursor ();

  std::shared_ptr <Tree> unaryItem = std::make_shared <Tree> ();
  unaryItem->_name = "UnaryItem";
  if (isLiteral (pig, "&") &&
      isUnaryItem (pig, unaryItem))
  {
    unaryItem->tag ("positive");
    unaryItem->tag ("lookahead");
    parseTree->addBranch (unaryItem);
    return true;
  }

  pig.restoreTo (checkpoint);
  if (isLiteral (pig, "!") &&
      isUnaryItem (pig, unaryItem))
  {
    unaryItem->tag ("negative");
    unaryItem->tag ("lookahead");
    parseTree->addBranch (unaryItem);
    return true;
  }

  pig.restoreTo (checkpoint);
  if (isUnaryItem (pig, unaryItem))
  {
    parseTree->addBranch (unaryItem);
    return true;
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// UnaryItem <-- PrimaryItem QUESTION / PrimaryItem STAR / PrimaryItem PLUS / PrimaryItem
bool Grammar::isUnaryItem (Pig& pig, std::shared_ptr <Tree> parseTree)
{
  std::shared_ptr <Tree> primaryItem = std::make_shared <Tree> ();
  primaryItem->_name = "PrimaryItem";
  if (isPrimaryItem (pig, primaryItem))
  {
         if (isLiteral (pig, "?")) primaryItem->tag ("question");
    else if (isLiteral (pig, "*")) primaryItem->tag ("star");
    else if (isLiteral (pig, "+")) primaryItem->tag ("plus");

    parseTree->addBranch (primaryItem);
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// PrimaryItem  <-- Identifier / CharLiteral / StringLiteral / OPEN Sequence? CLOSE
bool Grammar::isPrimaryItem (Pig& pig, std::shared_ptr <Tree> parseTree)
{
  std::shared_ptr <Tree> identifier = std::make_shared <Tree> ();
  identifier->_name = "Identifier";
  if (isIdentifier (pig, identifier))
  {
    parseTree->addBranch (identifier);
    return true;
  }

  std::shared_ptr <Tree> charLiteral = std::make_shared <Tree> ();
  charLiteral->_name = "CharLiteral";
  if (isCharLiteral (pig, charLiteral))
  {
    parseTree->addBranch (charLiteral);
    return true;
  }

  std::shared_ptr <Tree> stringLiteral = std::make_shared <Tree> ();
  stringLiteral->_name = "StringLiteral";
  if (isStringLiteral (pig, stringLiteral))
  {
    parseTree->addBranch (stringLiteral);
    return true;
  }

  auto checkpoint = pig.cursor ();
  if (isLiteral (pig, "("))
  {
    std::shared_ptr <Tree> sequence = std::make_shared <Tree> ();
    sequence->_name = "Sequence";
    if (! isSequence (pig, sequence))
      sequence->tag ("empty");

    if (isLiteral (pig, ")"))
    {
      parseTree->addBranch (sequence);
      return true;
    }
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Identifier <-- IdentStart IdentCont* Spacing
bool Grammar::isIdentifier (Pig& pig, std::shared_ptr <Tree> parseTree)
{
  auto checkpoint = pig.cursor ();
  if (isIdentStart (pig))
  {
    while (isIdentCont (pig))
    {
    }

    parseTree->attribute ("token", pig.substr (checkpoint, pig.cursor ()));

    isSpacing (pig);
    return true;
  }

  pig.restoreTo (checkpoint);
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
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// IdentCont <-- IdentStart / Digit
bool Grammar::isIdentCont (Pig& pig)
{
  if (isIdentStart (pig))
  {
    return true;
  }

  if (unicodeLatinDigit (pig.peek ()))
  {
    pig.skipN (1);
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// CharLiteral <-- ''' (!(''') QuotedChar) ''' Spacing
bool Grammar::isCharLiteral (Pig& pig, std::shared_ptr <Tree> parseTree)
{
  auto checkpoint = pig.cursor ();
  std::string value;
  if (pig.getQuoted ('\'', value) &&
      value.length () == 1        &&
      isSpacing (pig))
  {
    parseTree->attribute ("token", value);
    return true;
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// StringLiteral <-- '"' (!('"') QuotedChar)* '"' Spacing
bool Grammar::isStringLiteral (Pig& pig, std::shared_ptr <Tree> parseTree)
{
  std::string value;
  if (pig.getQuoted ('"', value) &&
      isSpacing (pig))
  {
    parseTree->attribute ("token", value);
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
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Grammar::isLiteral (
  Pig& pig,
  const std::string& thing)
{
  return pig.skipLiteral (thing) &&
         isSpacing (pig);
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

  if (value)
    std::cout << "Grammar debug mode.\n";
}

////////////////////////////////////////////////////////////////////////////////
std::string Grammar::dump () const
{
  return _rules->dump ();
}

////////////////////////////////////////////////////////////////////////////////
