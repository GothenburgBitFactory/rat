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

#include <Tree.h>
#include <FS.h>
#include <Pig.h>
#include <map>
#include <vector>
#include <string>

class Grammar
{
public:
  Grammar () = default;
  void initialize (const std::string&);
  void debug (bool);
  std::string dump () const;

private:
  bool isGrammar        (Pig&, std::shared_ptr <Tree>);
  bool isRule           (Pig&, std::shared_ptr <Tree>);
  bool isSequence       (Pig&, std::shared_ptr <Tree>);
  bool isAlternative    (Pig&, std::shared_ptr <Tree>);
  bool isUnaryItem      (Pig&, std::shared_ptr <Tree>);
  bool isPrimaryItem    (Pig&, std::shared_ptr <Tree>);
  bool isIdentifier     (Pig&, std::shared_ptr <Tree>);
  bool isIdentStart     (Pig&);
  bool isIdentCont      (Pig&);
  bool isCharLiteral    (Pig&, std::shared_ptr <Tree>);
  bool isStringLiteral  (Pig&, std::shared_ptr <Tree>);
  bool isSpacing        (Pig&);
  bool isSpaceChar      (Pig&);
  bool isLineComment    (Pig&);
  bool isLineTerminator (Pig&);
  bool isLiteral        (Pig&, const std::string&);
  bool isEOF            (Pig&);

private:
  bool                   _debug {false};
  std::string            _first {};
  std::shared_ptr <Tree> _rules {std::make_shared <Tree> ()};
};

#endif

////////////////////////////////////////////////////////////////////////////////
