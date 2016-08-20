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

#ifndef INCLUDED_GRAMMAR
#define INCLUDED_GRAMMAR

#include <FS.h>
#include <Pig.h>
#include <string>

class Grammar
{
public:
  Grammar () = default;
  void initialize (const std::string&);
  void debug (bool);
  std::string dump () const;

protected:
  enum class Quant { ZeroOrOne, ZeroOrMore, One, OneOrMore };

  class Token
  {
  public:
    Token (const std::string& value)         { _token      = value; }
    void quantifier (Grammar::Quant q)       { _quantifier = q;     }

    std::string    _token;
    Grammar::Quant _quantifier;
  };

/*
  class Production : public std::vector <Token>
  {
  };

  class Rule : public std::vector <Production>
  {
  };
*/

private:
  bool isGrammar        (Pig&);
  bool isRule           (Pig&);
  bool isSequence       (Pig&);
  bool isAlternative    (Pig&);
  bool isUnaryItem      (Pig&);
  bool isPrimaryItem    (Pig&);
  bool isIdentifier     (Pig&);
  bool isIdentStart     (Pig&);
  bool isIdentCont      (Pig&);
  bool isCharLiteral    (Pig&);
  bool isStringLiteral  (Pig&);
  bool isSpacing        (Pig&);
  bool isSpaceChar      (Pig&);
  bool isLineComment    (Pig&);
  bool isLineTerminator (Pig&);
  bool isLiteral        (Pig&, const std::string&);
  bool isEOF            (Pig&);

private:
  bool        _debug {false};
  std::string _first {};

  //           rule name    rule         production   decorated token
  //           |            |            |            |
  // std::map <std::string, std::vector <std::vector <Grammar::Token>>> _rules;
};

#endif
