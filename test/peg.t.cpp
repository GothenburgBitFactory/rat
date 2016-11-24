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
#include <test.h>

////////////////////////////////////////////////////////////////////////////////
int main (int, char**)
{
  UnitTest t (5);

  // Grammar with no input.
  try
  {
    PEG p;
    p.loadFromString ("");
    t.fail ("PEG: No input");
  }
  catch (const std::string& e) { t.is (e, "There are no rules defined.", "PEG: No input"); }

  // Grammar with no rules.
  try
  {
    PEG p;
    p.loadFromString ("# No grammar\n\n");
    t.fail ("PEG: Missing grammar, only comment");
  }
  catch (const std::string& e) { t.is (e, "There are no rules defined.", "PEG: Missing grammar, only comment"); }

  // Grammar with undefined reference.
  try
  {
    PEG p;
    p.loadFromString ("this: that");
    t.fail ("PEG: Grammar with undefined reference");
  }
  catch (const std::string& e) { t.is (e, "Definition 'that' referenced, but not defined.", "PEG: Grammar with undefined reference"); }

  // Grammar with left recursion.
  try
  {
    PEG p;
    p.loadFromString ("this: this");
    t.fail ("PEG: Grammar with left recursion");
  }
  catch (const std::string& e) { t.is (e, "Definition 'this' is left recursive.", "PEG: Grammar with left recursion"); }

  // Grammar with unreferenced definition.
  try
  {
    PEG p;
    p.loadFromString ("this: that\nthat: 'a'\nother: 'b'");
    t.fail ("PEG: Grammar with unreferenced definition");
  }
  catch (const std::string& e) { t.is (e, "Definition 'other' is defined, but not referenced.", "PEG: Grammar with unreferenced definition"); }

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
