////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2010 - 2016, Paul Beckingham, Federico Hernandez.
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
#include <Packrat.h>
#include <FS.h>
#include <cstdio>
#include <cstring>
#include <iostream>

////////////////////////////////////////////////////////////////////////////////
void usage ()
{
  std::cout << "\n"
            << "Usage: rat [options] <grammar> [<args>]\n"
            << "\n"
            << "Options are:\n"
            << "  -h/--help       Command usage\n"
            << "  -d/--debug      Debug mode\n"
            << "\n";
  exit (-1);
}

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char** argv)
{
  int status = 0;

  // Process command line arguments
  std::string grammarFile = "";
  std::string commandLine = "";
  bool debug = false;

  for (int i = 1; i < argc; ++i)
  {
    if (argv[i][0] == '-')
    {
           if (!strcmp (argv[i], "-h"))         usage ();
      else if (!strcmp (argv[i], "--help"))     usage ();
      else if (!strcmp (argv[i], "-d"))         debug = true;
      else if (!strcmp (argv[i], "--debug"))    debug = true;
      else
      {
        std::cout << "Unrecognized option '" << argv[i] << "'" << std::endl;
        usage ();
      }
    }
    else if (grammarFile == "")
    {
      grammarFile = argv[i];
    }
    else
    {
      if (commandLine != "")
        commandLine += " ";

      commandLine += "'" + std::string (argv[i]) + "'";
    }
  }

  // Display usage for incorrect command line.
  if (grammarFile == "")
    usage ();

  try
  {
    std::string contents;
    File (grammarFile).read (contents);

    Grammar grammar;
    grammar.debug (debug);
    grammar.initialize (contents);

    // Test commandLine against grammar.
    if (commandLine != "")
    {
      Packrat packrat;
      packrat.debug (debug);

      // TODO output = packrat (grammar, commandLine);
      // TODO dump output
    }
  }

  catch (const std::string& error)
  {
    std::cout << error << "\n";
    status = -1;
  }
  catch (...)
  {
    std::cout << "Unknown error\n";
    status = -2;
  }

  return status;
}

////////////////////////////////////////////////////////////////////////////////
