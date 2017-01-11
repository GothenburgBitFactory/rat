////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2010 - 2017, Paul Beckingham, Federico Hernandez.
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
#include <Timer.h>
#include <shared.h>
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
            << "  -s/--strict     Strict grammar validation\n"
            << "  -t/--time       Show performance metrics\n"
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
  args.addOption ("strict",  false);
  args.addOption ("time",    false);
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
    if (args.getOption ("strict"))
      peg.strict (true);
    for (auto i = 0; i < args.getOptionCount ("debug"); i++)
      peg.debug ();

    Timer t1;
    t1.start ();
    peg.loadFromString (grammar);
    t1.stop ();
    if (args.getOption ("time"))
      std::cout << "PEG load/parse " << t1.total_us () << "μs\n";

    // Gather all the entities.
    std::multimap <std::string, std::string> entities;
    for (int i = 1; i < args.getPositionalCount (); i++)
    {
      auto elements = split (args.getPositional (i), ':');
      if (elements[0] == "entity")
        entities.insert ({elements[1], elements[2]});
    }

    // Test commandLine against grammar.
    for (int i = 1; i < args.getPositionalCount (); i++)
    {
      auto arg = args.getPositional (i);
      if (arg.substr (0, 7) != "entity:")
      {
        // If the argument refers to an existing file, read it.
        File input (arg);
        if (input.exists () &&
            input.readable ())
          input.read (arg);

        // Create the parser.
        Packrat packrat;
        for (auto& entity : entities)
          packrat.entity (entity.first, entity.second);

        for (auto i = 0; i < args.getOptionCount ("debug"); i++)
          packrat.debug ();

        Timer t2;
        t2.start ();
        packrat.parse (peg, arg);
        t2.stop ();
        if (args.getOption ("time"))
          std::cout << "RAT parse " << t2.total_us () << "μs\n";

        std::cout << packrat.dump ();

        // TODO Ready for eval.
      }
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
