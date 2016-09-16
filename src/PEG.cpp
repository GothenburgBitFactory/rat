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
#include <PEG.h>
#include <Lexer.h>
#include <shared.h>
#include <format.h>
#include <iostream>
#include <sstream>

////////////////////////////////////////////////////////////////////////////////
void PEG::loadFromFile (File& file)
{
  if (! file.exists ())
    throw format ("PEG file '{1}' not found.", file._data);

  std::string contents;
  file.read (contents);
  loadFromString (contents);
}

////////////////////////////////////////////////////////////////////////////////
// Load and parse PEG.
//
// Syntax:
//   rule-name:  alternate1-token1 alternate1-token2
//               alternate2-token1
//
// - Rules are aligned at left margin only, followed by colon.
// - Productions are indented and never at left margin.
// - Blank lines delineate rules.
//
// Details:
// - Literals are always double-quoted.
// - "*", "+" and "?" suffixes have POSIX semantics.
// - "є" means empty set.
// - Literal modifiers:
//   - :a  Accept abbreviations
//   - :i  Accept caseless match
//
void PEG::loadFromString (const std::string& input)
{
  std::string rule_name = "";

  // This is a state machine.  Read each line.
  for (auto& line : split (input, '\n'))
  {
    // Skip whole-line comments, they have no semantics.
    if (line[0] == '#')
      continue;

    // Eliminate inline comments.
    std::string::size_type hash = line.find ('#');
    if (hash != std::string::npos)
      line.resize (hash);

    // Skip blank lines with no semantics.
    line = Lexer::trim (line);
    if (line == "" and rule_name == "")
      continue;

    if (line != "")
    {
      int token_count = 0;

      // Instantiate and configure the Lexer.
      Lexer l (line);
      l.noDate ();
      l.noDuration ();
      l.noUUID ();
      l.noHexNumber ();
      l.noURL ();
      l.noPath ();
      l.noPattern ();
      l.noOperator ();

      Lexer::Type type;
      std::string token;
      while (l.token (token, type))
      {
        ++token_count;

        // Rule definitions end in a colon.
        if (token.back () == ':')
        {
          // Capture the Rule_name.
          rule_name = token.substr (0, token.size () - 1);

          // If this is the first Rule, capture it as a starting point.
          if (_start == "")
            _start = rule_name;

          _rules[rule_name] = PEG::Rule ();
          token_count = 0;
        }
        // Production definition.
        else
        {
          // If no Production was added yet, add one.
          if (token_count <= 1)
            _rules[rule_name].push_back (PEG::Production ());

          // Decorate the token, if necessary.
          std::string::size_type start = 0;
          std::string::size_type end = token.length ();

          Token::Quantifier q = Token::Quantifier::one;
          Token::Lookahead l = Token::Lookahead::none;

          if (token.back () == '?')
          {
            q = Token::Quantifier::zero_or_one;
            --end;
          }
          else if (token.back () == '+')
          {
            q = Token::Quantifier::one_or_more;
            --end;
          }
          else if (token.back () == '*')
          {
            q = Token::Quantifier::zero_or_more;
            --end;
          }

          if (token.front () == '&')
          {
            l = Token::Lookahead::positive;
            ++start;
          }
          else if (token.front () == '!')
          {
            l = Token::Lookahead::negative;
            ++start;
          }

          PEG::Token t (token.substr (start, end - start));
          t._quantifier = q;
          t._lookahead = l;

          // Add the new Token to the most recent Production, of the current
          // Rule.
          _rules[rule_name].back ().push_back (t);
        }
      }
    }

    // A blank line in the input ends the current rule definition.
    else
      rule_name = "";
  }

  if (_debug)
    std::cout << dump () << "\n";

  // Validate the parsed grammar.
  validate ();
}

////////////////////////////////////////////////////////////////////////////////
std::map <std::string, PEG::Rule> PEG::syntax () const
{
  return _rules;
}

////////////////////////////////////////////////////////////////////////////////
std::string PEG::firstRule () const
{
  return _start;
}

////////////////////////////////////////////////////////////////////////////////
void PEG::debug (bool value)
{
  _debug = value;
}

////////////////////////////////////////////////////////////////////////////////
std::string PEG::dump () const
{
  std::stringstream out;
  out << "PEG\n";

  // Determine longest rule name, for display alignment.
  size_t longest = 0;
  for (const auto& rule : _rules)
    if (rule.first.length () > longest)
      longest = rule.first.length ();

  for (const auto& rule : _rules)
  {
    // Indicate the start Rule.
    out << "  "
        << (rule.first == _start ? "▶" : " ")
        << ' '
        << rule.first
        << ':'
        << std::string (1 + longest - rule.first.length (), ' ');

    int count = 0;
    for (const auto& production : rule.second)
    {
      if (count)
        out << std::string (6 + longest, ' ');

      for (const auto& token : production)
      {
        out << " "
            << (token._lookahead == Token::Lookahead::positive ? "&" : "")
            << (token._lookahead == Token::Lookahead::negative ? "!" : "")
            << token._token
            << (token._quantifier == Token::Quantifier::zero_or_one ? "?" : "")
            << (token._quantifier == Token::Quantifier::one_or_more ? "+" : "")
            << (token._quantifier == Token::Quantifier::zero_or_more ? "*" : "");
      }

      out << "\n";
      ++count;
    }

    out << "\n";
  }

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
void PEG::validate () const
{
  if (_start == "")
    throw std::string ("There are no rules defined.");

  std::vector <std::string> allRules;
  std::vector <std::string> allTokens;
  std::vector <std::string> allLeftRecursive;

  for (const auto& rule : _rules)
  {
    allRules.push_back (rule.first);

    for (const auto& production : rule.second)
    {
      for (const auto& token : production)
      {
        if (token._token.front () != '"' and  // Literals are not included.
            token._token.front () != '\'')    // Literals are not included.
          allTokens.push_back (token._token);

        if (token._token == production[0]._token and
            rule.first == production[0]._token   and
            production.size () == 1)
          allLeftRecursive.push_back (token._token);
      }
    }
  }

  std::vector <std::string> notUsed;
  std::vector <std::string> notDefined;
  listDiff (allRules, allTokens, notUsed, notDefined);

  // Undefined value - these are definitions that appear in token, but are
  // not in _rules.
  for (const auto& nd : notDefined)
    if (nd != "є")
      throw format ("Definition '{1}' referenced, but not defined.", nd);

  // Circular definitions - these are names in _rules that also appear as
  // the only token in any of the alternates for that definition.
  for (const auto& lr : allLeftRecursive)
    throw format ("Definition '{1}' is left recursive.", lr);

  for (const auto& r : allRules)
    if (r[0] == '"' or
        r[0] == '\'')
      throw format ("Definition '{1}' must not be a literal.");

  // Unused definitions - these are names in _rules that are never
  // referenced as token.
  for (const auto& nu : notUsed)
    if (nu != _start)
      throw format ("Definition '{1}' is defined, but not referenced.", nu);
}

////////////////////////////////////////////////////////////////////////////////