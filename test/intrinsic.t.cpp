////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2010 - 2016, Göteborg Bit Factory.
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
#include <Packrat.h>
#include <test.h>

////////////////////////////////////////////////////////////////////////////////
int main (int, char**)
{
  UnitTest t (11);

  // Grammar that is valid.
  PEG peg;
  peg.loadFromString ("thing: <character> <digit>");
  t.is (peg.firstRule (), "thing",                                          "intrinsic: firstRule found");

  auto rules = peg.syntax ();
  t.is (rules["thing"][0][0]._token,  "<character>",                        "intrinsic: thing: <character>");
  t.ok (rules["thing"][0][0]._quantifier == PEG::Token::Quantifier::one,    "intrinsic: thing: <character> quantifier one");
  t.ok (rules["thing"][0][0]._lookahead == PEG::Token::Lookahead::none,     "intrinsic: thing: <character> lookahead none");
  t.ok (rules["thing"][0][0]._tags == std::set <std::string> {"intrinsic"}, "intrinsic: thing: <character> tags {'intrinsic'}");

  t.is (rules["thing"][0][1]._token,  "<digit>",                            "intrinsic: thing: <digit>");
  t.ok (rules["thing"][0][1]._quantifier == PEG::Token::Quantifier::one,    "intrinsic: thing: <digit> quantifier one");
  t.ok (rules["thing"][0][1]._lookahead == PEG::Token::Lookahead::none,     "intrinsic: thing: <digit> lookahead none");
  t.ok (rules["thing"][0][1]._tags == std::set <std::string> {"intrinsic"}, "intrinsic: thing: <digit> tags {'intrinsic'}");

  // '12' is valid.
  try
  {
    Packrat rat;
    rat.parse (peg, "12");
    t.pass ("intrinsic: '12' valid");
  }
  catch (const std::string& e) { t.fail ("intrinsic: '12' " + e); }

  // '1x' is not valid.
  try
  {
    Packrat rat;
    rat.parse (peg, "1x");  // Expected to fail.
    t.fail ("intrinsic: '1x' not valid");
  }
  catch (const std::string& e) { t.pass ("intrinsic: '1x' " + e); }

  return 0;
}

////////////////////////////////////////////////////////////////////////////////