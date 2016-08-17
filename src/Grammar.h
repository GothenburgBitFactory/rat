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

private:
  bool isGrammar        (Pig&);
  bool isDefinition     (Pig&);
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
  bool isAnd            (Pig&);
  bool isNot            (Pig&);
  bool isQuestion       (Pig&);
  bool isStar           (Pig&);
  bool isPlus           (Pig&);
  bool isLeftArrow      (Pig&);
  bool isOpen           (Pig&);
  bool isSlash          (Pig&);
  bool isEOF            (Pig&);

private:
  bool        _debug {false};
  std::string _first {};
};

#endif
