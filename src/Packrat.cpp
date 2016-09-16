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
#include <iostream>

////////////////////////////////////////////////////////////////////////////////
void Packrat::debug (bool value)
{
  _debug = value;
}

////////////////////////////////////////////////////////////////////////////////
// Walk the grammar tree to parse the input text, resulting in a parse tree.
void Packrat::parse (const PEG& peg, const std::string& input)
{
  // Used to walk the grammar tree. Initially the top of the tree.
  auto syntax = peg.syntax ();
  auto first = peg.firstRule ();

  // The pig that will be sent down the pipe.
  Pig pig (input);

  // The resulting parse tree.
  std::shared_ptr <Tree> parseTree = std::make_shared <Tree> ();

  // Match the first rule.  Recursion does the rest.
  if (! matchRule (syntax, first, PEG::Token::Quantifier::one, pig, parseTree))
    throw std::string ("Parse failed.");
}

////////////////////////////////////////////////////////////////////////////////
// If there is a match, pig advances further down the pipe.
bool Packrat::matchRule (
  const std::map <std::string, PEG::Rule>& syntax,
  const std::string& rule,
  PEG::Token::Quantifier quantifier,
  Pig& pig,
  std::shared_ptr <Tree> parseTree)
{
  std::cout << "# ::matchRule " << rule << "\n";
  auto checkpoint = pig.cursor ();




  pig.restoreTo (checkpoint);
  return false;
}

/*
bool Packrat::matchRuleQuant (rule, quantifier, pig, p)
{
  return false;
}
bool Packrat::matchProduction (rule, quantifier, production_index, pig, p)
{
  return false;
}
bool Packrat::matchToken (rule, quantifier, production_index, token, pig, p)
{
  return false;
}
*/

////////////////////////////////////////////////////////////////////////////////
std::string Packrat::dump () const
{
  return "";
}

////////////////////////////////////////////////////////////////////////////////
