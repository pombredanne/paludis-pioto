/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
 *
 * This file is part of the Paludis package manager. Paludis is free software;
 * you can redistribute it and/or modify it under the terms of the GNU General
 * Public License version 2, as published by the Free Software Foundation.
 *
 * Paludis is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "merge_common.hh"

#include <paludis/digests/md5.hh>

#include <paludis/util/dir_iterator.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/log.hh>
#include <paludis/util/pstream.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/system.hh>
#include <paludis/util/tokeniser.hh>

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>
#include <list>

#include <cstdlib>
#include <errno.h>
#include <fcntl.h>
#include <fnmatch.h>
#include <sys/stat.h>
#include <sys/types.h>

using namespace paludis;
using namespace merge;

using std::cout;
using std::cerr;
using std::endl;
using std::ifstream;

namespace
{
    int exit_status;

    template <typename Iter_>
    void unmerge_contents(const FSEntry & root, const Iter_ begin, const Iter_ end)
    {
        using std::istreambuf_iterator;

        for (Iter_ cur(begin) ; cur != end ; ++cur)
        {
            std::vector<std::string> tokens;
            WhitespaceTokeniser::get_instance()->tokenise(*cur, std::back_inserter(tokens));
            if (tokens.empty())
                continue;

            if ("obj" == tokens.at(0))
            {
                while (tokens.size() > 4)
                {
                    if (std::string::npos != tokens.at(4).find('='))
                        break;

                    tokens.at(1).append(" " + tokens.at(2));
                    for (unsigned i = 2 ; i < tokens.size() - 1 ; ++i)
                        tokens.at(i) = tokens.at(i + 1);
                    tokens.pop_back();
                }

                if (tokens.size() != 4)
                {
                    Log::get_instance()->message(ll_warning, lc_no_context,
                            "Malformed VDB entry '" + *cur + "'");
                    exit_status |= 4;
                }
                else if (! (root / tokens.at(1)).is_regular_file())
                    cout << "--- [!type] " << tokens.at(1) << endl;
                else if (stringify((root / tokens.at(1)).mtime()) != tokens.at(3))
                    cout << "--- [!time] " << tokens.at(1) << endl;
                else
                {
                    ifstream md5_file(stringify(root / tokens.at(1)).c_str());
                    if (! md5_file)
                    {
                        Log::get_instance()->message(ll_warning, lc_no_context,
                                "Couldn't get md5 for '" + stringify(root / tokens.at(1)) + "'");
                        cout << "--- [!md5?] " << tokens.at(1) << endl;
                    }
                    else if (MD5(md5_file).hexsum() != tokens.at(2))
                        cout << "--- [!md5 ] " << tokens.at(1) << endl;
                    else if (is_config_protected(root, root / tokens.at(1)))
                        cout << "--- [cfgpr] " << tokens.at(1) << endl;
                    else
                    {
                        cout << "<<<         " << tokens.at(1) << endl;

                        mode_t mode((root / tokens.at(1)).permissions());
                        if ((mode & S_ISUID) || (mode & S_ISGID))
                        {
                            mode &= 0400;
                            (root / tokens.at(1)).chmod(mode);
                        }
                        (root / tokens.at(1)).unlink();
                    }
                }
            }
            else if ("sym" == tokens.at(0))
            {
                while (tokens.size() > 5)
                {
                    if (std::string::npos != tokens.at(2).find('='))
                        break;

                    if (tokens.at(2) == "->")
                        break;

                    tokens.at(1).append(" " + tokens.at(2));
                    for (unsigned i = 2 ; i < tokens.size() - 1; ++i)
                        tokens.at(i) = tokens.at(i + 1);
                    tokens.pop_back();
                }

                while (tokens.size() > 5)
                {
                    if (std::string::npos != tokens.at(2).find('='))
                        break;

                    if (tokens.at(4) == "->")
                        break;

                    tokens.at(3).append(" " + tokens.at(4));
                    for (unsigned i = 4 ; i < tokens.size() - 1; ++i)
                        tokens.at(i) = tokens.at(i + 1);
                    tokens.pop_back();
                }

                if (tokens.size() != 5)
                {
                    Log::get_instance()->message(ll_warning, lc_no_context,
                            "Malformed VDB entry '" + *cur + "'");
                    exit_status |= 4;
                }
                else if (! (root / tokens.at(1)).is_symbolic_link())
                    cout << "--- [!type] " << tokens.at(1) << endl;
                else if (stringify((root / tokens.at(1)).mtime()) != tokens.at(4))
                    cout << "--- [!time] " << tokens.at(1) << endl;
                else if ((root / tokens.at(1)).readlink() != tokens.at(3))
                    cout << "--- [!dest] " << tokens.at(1) << endl;
                else
                {
                    cout << "<<<         " << tokens.at(1) << endl;
                    (root / tokens.at(1)).unlink();
                }
            }
            else if ("misc" == tokens.at(0))
            {
            }
            else if ("fif" == tokens.at(0) || "dev" == tokens.at(0))
            {
                while (tokens.size() > 2)
                {
                    if (std::string::npos != tokens.at(2).find('='))
                        break;

                    tokens.at(1).append(" " + tokens.at(2));
                    for (unsigned i = 2 ; i < tokens.size() - 1; ++i)
                        tokens.at(i) = tokens.at(i + 1);
                    tokens.pop_back();
                }

                if (tokens.size() != 2)
                {
                    Log::get_instance()->message(ll_warning, lc_no_context,
                            "Malformed VDB entry '" + *cur + "'");
                    exit_status |= 8;
                }
                else if ("fif" == tokens.at(0) && ! (root / tokens.at(1)).is_fifo())
                    cout << "--- [!type] " << tokens.at(1) << endl;
                else if ("dev" == tokens.at(0) && ! (root / tokens.at(1)).is_device())
                    cout << "--- [!type] " << tokens.at(1) << endl;
                else
                {
                    cout << "<<<         " << tokens.at(1) << endl;
                    (root / tokens.at(1)).unlink();
                }
            }
            else if ("dir" == tokens.at(0))
                /* nothing */ ;
            else
            {
                Log::get_instance()->message(ll_warning, lc_no_context,
                        "Skipping unknown VDB entry '" + *cur + "'");
                exit_status |= 2;
            }
        }
    }

    template <typename Iter_>
    void unmerge_directories(const FSEntry & root, const Iter_ begin, const Iter_ end)
    {
        using std::istreambuf_iterator;

        for (Iter_ cur(begin) ; cur != end ; ++cur)
        {
            std::vector<std::string> tokens;
            WhitespaceTokeniser::get_instance()->tokenise(*cur, std::back_inserter(tokens));
            if (tokens.empty())
                continue;

            if ("dir" == tokens.at(0))
            {
                while (tokens.size() > 2)
                {
                    if (std::string::npos != tokens.at(2).find('='))
                        break;

                    tokens.at(1).append(" " + tokens.at(2));
                    for (unsigned i = 2 ; i < tokens.size() - 1; ++i)
                        tokens.at(i) = tokens.at(i + 1);
                    tokens.pop_back();
                }

                if (tokens.size() != 2)
                {
                    Log::get_instance()->message(ll_warning, lc_no_context,
                            "Malformed VDB entry '" + *cur + "'");
                    exit_status |= 8;
                }
                else if (! (root / tokens.at(1)).is_directory())
                    cout << "--- [!type] " << tokens.at(1) << endl;
                else if (DirIterator((root / tokens.at(1)), false) != DirIterator())
                    cout << "--- [!empt] " << tokens.at(1) << endl;
                else
                {
                    cout << "<<<         " << tokens.at(1) << endl;
                    (root / tokens.at(1)).rmdir();
                }
            }
        }
    }
}

int
main(int argc, char * argv[])
{
    Context context("In main program:");
    exit_status = 0;
    try
    {
        if (argc != 3)
            throw Failure("Usage: " + stringify(argv[0]) + " root contents");

        Log::get_instance()->set_program_name(argv[0]);
        std::string log_level(getenv_with_default("PALUDIS_EBUILD_LOG_LEVEL", "qa"));

        if (log_level == "debug")
            Log::get_instance()->set_log_level(ll_debug);
        else if (log_level == "qa")
            Log::get_instance()->set_log_level(ll_qa);
        else if (log_level == "warning")
            Log::get_instance()->set_log_level(ll_warning);
        else if (log_level == "silent")
            Log::get_instance()->set_log_level(ll_silent);
        else
            throw Failure("bad value for log level");

        FSEntry root(argv[1]), contents(argv[2]);

        if (! ((root = root.realpath())).is_directory())
            throw Failure(stringify(argv[1]) + ": not a directory");

        ifstream contents_file(stringify(contents).c_str());
        if (! contents_file)
            throw Failure(stringify(contents) + ": not readable");

        std::list<std::string> lines;
        std::string line;
        while (std::getline(contents_file, line))
            lines.push_back(line);

        unmerge_contents(root, lines.begin(), lines.end());
        unmerge_directories(root, lines.rbegin(), lines.rend());
        return exit_status;
    }
    catch (const Failure & f)
    {
        cerr << argv[0] << ": fatal error: " << f.message << endl;
        return EXIT_FAILURE;
    }
    catch (const Exception & e)
    {
        cerr << argv[0] << ": fatal error:" << endl
            << "  * " << e.backtrace("\n  * ") << e.message()
            << " (" << e.what() << ")" << endl;
        return EXIT_FAILURE;
    }
    catch (const std::exception & e)
    {
        cerr << argv[0] << ": fatal error: " << e.what() << endl;
        return EXIT_FAILURE;
    }
}

