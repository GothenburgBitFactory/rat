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
#include <PEG.h>
#include <Packrat.h>
#include <Args.h>
#include <FS.h>
#include <cstdio>
#include <cstring>
#include <iostream>

////////////////////////////////////////////////////////////////////////////////
void usage ()
{
  std::cout << '\n'
            << "Usage: rat [options] <grammar> [<args>]\n"
            << '\n'
            << "Options are:\n"
            << "  -v/--version    Version number\n"
            << "  -h/--help       Command usage\n"
            << "  -d/--debug      Debug mode\n"
            << '\n';
  exit (0);
}

////////////////////////////////////////////////////////////////////////////////
int main (int argc, const char* argv[])
{
  int status = 0;

  // Process command line arguments
  Args args;
  args.addOption ("help",    false);
  args.addOption ("version", false);
  args.addOption ("debug",   false);
  args.scan (argc, argv);

  if (args.getOption ("help"))
    usage ();

  if (args.getOption ("version"))
  {
    std::cout << VERSION << '\n';
    exit (0);
  }

  if (args.getPositionalCount () < 2)
    usage ();

  try
  {
    // Read the grammar file.
    std::string grammar;
    File (args.getPositional (0)).read (grammar);

    // Parse the grammar.
    PEG peg;
    if (args.getOption ("debug"))
      peg.debug ();
    peg.loadFromString (grammar);

    // Test commandLine against grammar.
    for (int i = 1; i < args.getPositionalCount (); i++)
    {
      auto arg = args.getPositional (i);

      // If the argument refers to an existing file, read it.
      File input (arg);
      if (input.exists () &&
          input.readable ())
        input.read (arg);

      // Create the parser.
      Packrat packrat;
      packrat.debug (args.getOption ("debug"));
      packrat.parse (peg, arg);
      std::cout << packrat.dump ();

      // TODO Ready for eval.
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
