/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaranm@gentoo.org>
 * Copyright (c) 2006 Mark Loeser <halcy0n@gentoo.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_FS_ENTRY_HH
#define PALUDIS_GUARD_PALUDIS_FS_ENTRY_HH 1

#include <string>
#include <ostream>
#include <paludis/comparison_policy.hh>
#include <paludis/exception.hh>
#include <paludis/counted_ptr.hh>
#include <sys/stat.h>

/** \file
 * Declarations for paludis::Filesystem.
 *
 * \ingroup Filesystem
 * \ingroup Exception
 */

namespace paludis
{
    /**
     * Generic filesystem error class.
     *
     * \ingroup Exception
     */
    class FSError : public Exception
    {
        public:
            /**
             * Constructor.
             */
            FSError(const std::string & message) throw ();
    };

    /**
     * File permissions used by FSEntry
     */
    enum FSPermission
    {
        fs_perm_read,       ///< read permission on file
        fs_perm_write,      ///< write permission on file
        fs_perm_execute     ///< execute permission on file
    };

    /**
     * User classes used by FSEntry
     */
    enum FSUserGroup
    {
        fs_ug_owner,         ///< owner permission
        fs_ug_group,        ///< group permission
        fs_ug_others        ///< others permission
    };

    /**
     * Represents an entry (which may or may not exist) in the filesystem.
     *
     * \ingroup Filesystem
     */
    class FSEntry : public ComparisonPolicy<
                        FSEntry,
                        comparison_mode::FullComparisonTag,
                        comparison_method::CompareByMemberTag<std::string> >
    {
        private:
            std::string _path;

            mutable CountedPtr<struct stat, count_policy::ExternalCountTag> _stat_info;

            mutable bool _exists;

            /**
             * Whether or not we have run _stat() on this location yet
             */
            mutable bool _checked;

            void _normalise();

            void _stat() const;

        public:
            /**
             * Constructor, from a path.
             */
            FSEntry(const std::string & path);

            /**
             * Copy constructor.
             */
            FSEntry(const FSEntry & other);

            /**
             * Destructor.
             */
            ~FSEntry();

            /**
             * Assignment, from another FSEntry.
             */
            const FSEntry & operator= (const FSEntry & other);

            /**
             * Join with another FSEntry.
             */
            FSEntry operator/ (const FSEntry & rhs) const;

            /**
             * Append another FSEntry.
             */
            const FSEntry & operator/= (const FSEntry & rhs);

            /**
             * Join with another path.
             */
            FSEntry operator/ (const std::string & rhs) const;

            /**
             * Append another path.
             */
            const FSEntry & operator/= (const std::string & rhs)
            {
                return operator/= (FSEntry(rhs));
            }

            /**
             * Fetch a string representation of our path.
             */
            operator std::string() const;

            /**
             * Does a filesystem entry exist at our location?
             */
            bool exists() const;

            /**
             * Does a filesystem entry exist at our location, and if it does,
             * is it a directory?
             */
            bool is_directory() const;

            /**
             * Does a filesystem entry exist at our location, and if it does,
             * is it a regular file?
             */
            bool is_regular_file() const;

            /**
             * Check if filesystem entry has `perm` for `user_group`
             */
            bool has_permission(const FSUserGroup & user_group, const FSPermission & fs_perm) const;

            /**
             * Return the last part of our path (eg '/foo/bar' => 'bar').
             */
            std::string basename() const;

            /**
             * Return the canonicalised version of our path.
             */
            FSEntry realpath() const;

            /**
             * Return the time the filesystem entry was created
             */
            time_t ctime() const;

            /**
             * Return the time the filesystem entry was last modified
             */
            time_t mtime() const;
    };

    /**
     * An FSEntry can be written to an ostream.
     */
    std::ostream & operator<< (std::ostream & s, const FSEntry & f);
}

#endif
