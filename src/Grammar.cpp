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

    // Skip blank lines with no semantics.
    line = Lexer::trim (line);
    if (line == "" and rule_name == "")
      continue;

    if (line != "")
    {
      std::cout << "# Grammar line " << line << "\n";

      Lexer l (line);
      l.noPattern ();
      l.noPath ();
      l.noUUID ();
      l.noDate ();
      l.noDuration ();

      auto tokens = l.tokenize (line);
      if (tokens.size () < 5)
        throw std::string ("Incomplete line: ") + line;

      // Capture rule name, and first rule (entry point).
      rule_name = std::get <0> (tokens[0]);
      if (_first == "")
        _first = rule_name;

      // rule < - - ...
      // 0    1 2 3
      unsigned int i = 4;
      while (i < tokens.size ())
      {
        // TODO Recognize all the following possibilities:
        // TODO Should this instead be a recursive descent parser?

        // TODO ()
        // TODO (e)
        // TODO nonterminal
        // TODO literal
        // TODO choice
        // TODO optional?
        // TODO any*
        // TODO mandatory+
        // TODO &(positive_lookahead)
        // TODO !(negative_lookahead)
      }

      // TODO Store something indexed by rule_name.
    }
  }

  if (_debug)
    std::cout << dump () << "\n";

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
