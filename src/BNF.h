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

#ifndef INCLUDED_BNF
#define INCLUDED_BNF

#include <FS.h>
#include <string>
#include <vector>
#include <map>

class BNF
{
public:
  class Token
  {
  public:
    Token (const std::string& value)         { _token      = value; }
    void decorate (const std::string& value) { _decoration = value; }

    std::string _token;
    std::string _decoration;
  };

  class Production : public std::vector <Token>
  {
  };

  class Rule : public std::vector <Production>
  {
  };

public:
  void loadFromFile (File&);
  void loadFromString (const std::string&);
  std::map <std::string, BNF::Rule> syntax () const;
  void debug (bool);
  std::string dump () const;

private:
  void validate () const;

private:
  //        rule name    rule
  //        |            |
  std::map <std::string, BNF::Rule> _rules {};
  std::string                       _start {};
  bool                              _debug {false};
};

#endif
