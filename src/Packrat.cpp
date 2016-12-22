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
#include <Packrat.h>
#include <shared.h>
#include <format.h>
#include <unicode.h>
#include <utf8.h>
#include <iostream>

int Packrat::minimumMatchLength = 3;

////////////////////////////////////////////////////////////////////////////////
void Packrat::debug (bool value)
{
  _debug = value;
}

////////////////////////////////////////////////////////////////////////////////
// Walk the grammar tree to parse the input text, resulting in a parse tree.
void Packrat::parse (const PEG& peg, const std::string& input)
{
  // Used to walk the grammar tree.
  // Note there is only one rule at the top of the syntax tree, which was the
  // first one defined.
  _syntax = peg.syntax ();
  auto first = peg.firstRule ();

  // The pig that will be sent down the pipe.
  Pig pig (input);

  // Name the root node.
  _tree->_name = "Root";

  // Match the first rule.  Recursion does the rest.
  if (! matchRule (first, pig, _tree))
    throw std::string ("Parse failed.");

  if (! pig.eos ())
    throw format ("Parse failed - extra character at position {1}.", pig.cursor ());
}

////////////////////////////////////////////////////////////////////////////////
void Packrat::entity (const std::string& category, const std::string& name)
{
  // Walk the list of entities for category.
  auto c = _entities.equal_range (category);
  for (auto e = c.first; e != c.second; ++e)
    if (e->second == name)
      return;

  // The category/name pair was not found, therefore add it.
  _entities.insert (std::pair <std::string, std::string> (category, name));
}

////////////////////////////////////////////////////////////////////////////////
// If there is a match, pig advances further down the pipe.
bool Packrat::matchRule (
  const std::string& rule,
  Pig& pig,
  std::shared_ptr <Tree> parseTree)
{
  if (_debug)
    std::cout << "trace matchRule " << rule << "\n";
  auto checkpoint = pig.cursor ();

  for (const auto& production : _syntax.find (rule)->second)
    if (matchProduction (production, pig, parseTree))
      return true;

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Packrat::matchProduction (
  const PEG::Production& production,
  Pig& pig,
  std::shared_ptr <Tree> parseTree)
{
  if (_debug)
    std::cout << "trace   matchProduction\n";
  auto checkpoint = pig.cursor ();

  auto collector = std::make_shared <Tree> ();
  for (const auto& token : production)
  {
    auto b = std::make_shared <Tree> ();
    if (! matchTokenQuant (token, pig, b))
    {
      pig.restoreTo (checkpoint);
      return false;
    }

    // Accumulate branches.
    collector->addBranch (b);
  }

  // On success transfer all sub-branches.
  for (auto& b : collector->_branches)
    for (auto sub : b->_branches)
      parseTree->addBranch (sub);

  return true;
}

////////////////////////////////////////////////////////////////////////////////
// Wraps calls to matchTokenLookahead, while properly handling the quantifier.
bool Packrat::matchTokenQuant (
  const PEG::Token& token,
  Pig& pig,
  std::shared_ptr <Tree> parseTree)
{
  if (_debug)
    std::cout << "trace     matchToken " << token.dump () << "\n";

  // Must match exactly once, so run once and return the result.
  if (token._quantifier == PEG::Token::Quantifier::one)
  {
    return matchTokenLookahead (token, pig, parseTree);
  }

  // May match zero or one time.  If it matches, the cursor will be advanced.
  // If it fails, the cursor will not be advanced, but this is still considered
  // successful.  Return true either way, but backtrack the cursor on failure.
  else if (token._quantifier == PEG::Token::Quantifier::zero_or_one)
  {
    // Check for a single match, succeed anyway.
    matchTokenLookahead (token, pig, parseTree);
    return true;
  }

  // May match 1 or more times.  If it matches on the first attempt, continue
  // to greedily match until it fails.  If it fails on the first attempt, then
  // the rule fails.
  else if (token._quantifier == PEG::Token::Quantifier::one_or_more)
  {
    if (! matchTokenLookahead (token, pig, parseTree))
      return false;

    while (matchTokenLookahead (token, pig, parseTree))
    {
      // "Forget it, he's rolling."
    }

    return true;
  }

  // May match zero or more times.  Keep calling while there are matches, and
  // return true always.  Backtrack the cursor on failure.
  else if (token._quantifier == PEG::Token::Quantifier::zero_or_more)
  {
    while (matchTokenLookahead (token, pig, parseTree))
    {
      // Let it go.
    }

    return true;
  }

  throw std::string ("This should never happen.");
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Wraps calls to matchToken, while properly handling lookahead.
bool Packrat::matchTokenLookahead (
  const PEG::Token& token,
  Pig& pig,
  std::shared_ptr <Tree> parseTree)
{
  if (token._lookahead == PEG::Token::Lookahead::none)
  {
    return matchToken (token, pig, parseTree);
  }
  else if (token._lookahead == PEG::Token::Lookahead::positive)
  {
    auto checkpoint = pig.cursor ();
    auto b = std::make_shared <Tree> ();
    if (matchToken (token, pig, b))
    {
      pig.restoreTo (checkpoint);
      return true;
    }
  }
  else if (token._lookahead == PEG::Token::Lookahead::negative)
  {
    auto checkpoint = pig.cursor ();
    auto b = std::make_shared <Tree> ();
    if (! matchToken (token, pig, b))
    {
      return true;
    }

    pig.restoreTo (checkpoint);
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Packrat::matchToken (
  const PEG::Token& token,
  Pig& pig,
  std::shared_ptr <Tree> parseTree)
{
  auto checkpoint = pig.cursor ();
  auto b = std::make_shared <Tree> ();

  if (token.hasTag ("intrinsic") &&
      matchIntrinsic (token, pig, parseTree))
  {
    return true;
  }

  else if (_syntax.find (token._token) != _syntax.end () &&
           matchRule (token._token, pig, b))
  {
    // This is the only case that adds a sub-branch.
    b->_name = token._token;
    parseTree->addBranch (b);
    return true;
  }

  else if (token.hasTag ("literal") &&
           token.hasTag ("character") &&
           matchCharLiteral (token, pig, parseTree))
  {
   return true;
  }

  else if (token.hasTag ("literal") &&
           token.hasTag ("string") &&
           matchStringLiteral (token, pig, parseTree))
  {
    return true;
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Supports the following:
//   <digit>      --> unicodeLatinDigit
//   <character>  --> anything
//   <punct>      --> unicodePunctuation
//   <eol>        --> unicodeVerticalWhitespace
bool Packrat::matchIntrinsic (
  const PEG::Token& token,
  Pig& pig,
  std::shared_ptr <Tree> parseTree)
{
  if (_debug)
    std::cout << "trace       matchIntrinsic " << token.dump () << "\n";
  auto checkpoint = pig.cursor ();

  if (token._token == "<digit>")
  {
    int digit;
    if (pig.getDigit (digit))
    {
      // Create a populated branch.
      auto b = std::make_shared <Tree> ();
      b->_name = "intrinsic";
      b->attribute ("expected", token._token);
      b->attribute ("value", format ("{1}", digit));
      parseTree->addBranch (b);

      if (_debug)
        std::cout << "trace         [32mmatch[0m " << digit << "\n";
      return true;
    }
  }
  else if (token._token == "<character>")
  {
    int character;
    if (pig.getCharacter (character))
    {
      // Create a populated branch.
      auto b = std::make_shared <Tree> ();
      b->_name = "intrinsic";
      b->attribute ("expected", token._token);
      b->attribute ("value", format ("{1}", character));
      parseTree->addBranch (b);

      if (_debug)
        std::cout << "trace         [32mmatch[0m " << character << "\n";
      return true;
    }
  }

  // <punct> ::ispunct
  else if (token._token == "<punct>")
  {
    int character = pig.peek ();
    if (unicodePunctuation (character))
    {
      pig.skip (character);

      // Create a populated branch.
      auto b = std::make_shared <Tree> ();
      b->_name = "intrinsic";
      b->attribute ("expected", token._token);
      b->attribute ("value", format ("{1}", character));
      parseTree->addBranch (b);

      if (_debug)
        std::cout << "trace         [32mmatch[0m " << character << "\n";
      return true;
    }
  }

  // <eol>
  else if (token._token == "<eol>")
  {
    int character = pig.peek ();
    if (unicodeVerticalWhitespace (character))
    {
      pig.skip (character);

      // Create a populated branch.
      auto b = std::make_shared <Tree> ();
      b->_name = "intrinsic";
      b->attribute ("expected", token._token);
      b->attribute ("value", format ("{1}", character));
      parseTree->addBranch (b);

      if (_debug)
        std::cout << "trace         [32mmatch[0m " << character << "\n";
      return true;
    }
  }

  if (_debug)
     std::cout << "trace         [31mfail[0m " << token._token << "\n";
  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Packrat::matchCharLiteral (
  const PEG::Token& token,
  Pig& pig,
  std::shared_ptr <Tree> parseTree)
{
  if (_debug)
    std::cout << "trace       matchCharLiteral " << token.dump () << "\n";
  auto checkpoint = pig.cursor ();

  if (token._token.length () >= 3 &&
      token._token[0] == '\'' &&
      token._token[2] == '\'')
  {
    int literal = token._token[1];
    if (pig.skip (literal))
    {
      // Create a populated branch.
      auto b = std::make_shared <Tree> ();
      b->_name = "charLiteral";
      b->attribute ("expected", token._token);
      b->attribute ("value", utf8_character (literal));
      parseTree->addBranch (b);

      if (_debug)
        std::cout << "trace         [32mmatch[0m " << token._token << "\n";
      return true;
    }
  }

  if (_debug)
    std::cout << "trace         [31mfail[0m " << token._token << "\n";
  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Packrat::matchStringLiteral (
  const PEG::Token& token,
  Pig& pig,
  std::shared_ptr <Tree> parseTree)
{
  if (_debug)
    std::cout << "trace       matchStringLiteral " << token.dump () << "\n";
  auto checkpoint = pig.cursor ();

  std::string literal = token._token.substr (1, token._token.length () - 2);
  if (pig.skipLiteral (literal))
  {
    // Create a populated branch.
    auto b = std::make_shared <Tree> ();
    b->_name = "stringLiteral";
    b->attribute ("expected", token._token);
    b->attribute ("value", literal);
    parseTree->addBranch (b);

    if (_debug)
      std::cout << "trace         [32mmatch[0m " << literal << "\n";
    return true;
  }

  if (_debug)
    std::cout << "trace         [31mfail[0m " << token._token << "\n";
  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Search for 'value' in _entities category, return canonicalized value.
bool Packrat::canonicalize (
  std::string& canonicalized,
  const std::string& category,
  const std::string& value) const
{
  // Extract a list of entities for category.
  std::vector <std::string> options;
  auto c = _entities.equal_range (category);
  for (auto e = c.first; e != c.second; ++e)
  {
    // Shortcut: if an exact match is found, success.
    if (value == e->second)
    {
      canonicalized = value;
      return true;
    }

    options.push_back (e->second);
  }

  // Match against the options, throw away results.
  std::vector <std::string> matches;
  if (autoComplete (value, options, matches, minimumMatchLength) == 1)
  {
    canonicalized = matches[0];
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
std::string Packrat::dump () const
{
  std::stringstream out;
  if (_debug)
    out << '\n';

  out << "Packrat Parse "
      << _tree->dump ();

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
