#!/usr/bin/env python
# vim: set fileencoding=utf-8 sw=4 sts=4 et :

#
# Copyright (c) 2007 Piotr Jaroszyński <peper@gentoo.org>
#
# This file is part of the Paludis package manager. Paludis is free software;
# you can redistribute it and/or modify it under the terms of the GNU General
# Public License version 2, as published by the Free Software Foundation.
#
# Paludis is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along with
# this program; if not, write to the Free Software Foundation, Inc., 59 Temple
# Place, Suite 330, Boston, MA  02111-1307  USA
#

from paludis import *
import unittest

class TestCase_Names(unittest.TestCase):
    def test_1_create(self):
        self.names = {}
        self.names["cat-foo"] = CategoryNamePart("cat-foo")
        self.names["pkg"] = PackageNamePart("pkg")
        self.names["cat-foo/pkg"] = QualifiedPackageName(self.names["cat-foo"], self.names["pkg"])
        self.names["cat-blah/pkg"] = QualifiedPackageName("cat-blah/pkg")
        self.names["useflag"] = UseFlagName("useflag")
        self.names["3.3"] = SlotName("3.3")
        self.names["repo"] = RepositoryName("repo")
        self.names["keyword"] = KeywordName("keyword")
        self.names["set"] = SetName("set")
        IUseFlag("foo", IUseFlagParseMode.PERMISSIVE)
        IUseFlag("foo", UseFlagState.ENABLED)

    def test_2_create_error(self):
        self.assertRaises(PackageNamePartError, PackageNamePart, ":bad")
        self.assertRaises(CategoryNamePartError, CategoryNamePart, ":bad")
        self.assertRaises(QualifiedPackageNameError, QualifiedPackageName, ":bad")
        self.assertRaises(UseFlagNameError, UseFlagName, "-bad")
        self.assertRaises(SlotNameError, SlotName, ":bad")
        self.assertRaises(RepositoryNameError, RepositoryName, ":bad")
        self.assertRaises(KeywordNameError, KeywordName, ":bad")
        self.assertRaises(SetNameError, SetName, ":bad")
        self.assertRaises(Exception, PackageNamePartCollection)
        self.assertRaises(Exception, CategoryNamePartCollection)
        self.assertRaises(Exception, QualifiedPackageNameCollection)
        self.assertRaises(Exception, UseFlagNameCollection)
        self.assertRaises(Exception, RepositoryNameCollection)

    def test_3_str(self):
        self.test_1_create()
        for (k, v) in self.names.items():
            self.assertEqual(str(v), k)

        self.assertEqual(str(IUseFlag("foo", UseFlagState.ENABLED)), "+foo")

    def test_4_operators(self):
        self.assert_(CategoryNamePart("cat-foo") + PackageNamePart("pkg") == QualifiedPackageName("cat-foo/pkg"))
        self.assert_(IUseFlag("foo", UseFlagState.ENABLED) == IUseFlag("+foo", IUseFlagParseMode.PERMISSIVE))


    def test_5_data_members(self):
        qpn = QualifiedPackageName("cat/foo")
        self.assertEqual(str(qpn.category), "cat")
        self.assertEqual(str(qpn.package), "foo")
        qpn.category = "blah"
        qpn.package = "bar"
        self.assertEqual(str(qpn.category), "blah")
        self.assertEqual(str(qpn.package), "bar")

        iuf = IUseFlag("foo", UseFlagState.ENABLED)
        iuf.flag = "blah"
        iuf.state = UseFlagState.DISABLED
        self.assertEqual(str(iuf), "-blah")

if __name__ == "__main__":
    unittest.main()
