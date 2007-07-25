#!/usr/bin/env ruby
# vim: set sw=4 sts=4 et tw=80 :

#
# Copyright (c) 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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

require 'test/unit'
require 'Paludis'

module Paludis
    class TestCase_QACheckProperties < Test::Unit::TestCase
        def test_create
            m = QACheckProperties.new
        end

        def test_each
            m = QACheckProperties.new
            assert_equal [], m.to_a
        end

        def test_empty
            m = QACheckProperties.new
            assert m.empty?
        end

        def test_set
            m = QACheckProperties.new
            m.set QACheckProperty::NeedsBuild
            m.set QACheckProperty::NeedsNetwork
            :qa

            assert ! m.empty?

            assert m.include?(QACheckProperty::NeedsBuild)
            assert m.include?(QACheckProperty::NeedsNetwork)
        end
    end

    class TestCase_QAReporter < Test::Unit::TestCase
        def test_respond
            assert_respond_to QAReporter.new, :message
        end
    end
end
