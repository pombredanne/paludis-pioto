#!/bin/bash
# vim: set sw=4 sts=4 et :

# Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
#
# Based in part upon ebuild.sh from Portage, which is Copyright 1995-2005
# Gentoo Foundation and distributed under the terms of the GNU General
# Public License v2.
#
# This file is part of the Paludis package manager. Paludis is free software;
# you can redistribute it and/or modify it under the terms of the GNU General
# Public License, version 2, as published by the Free Software Foundation.
#
# Paludis is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along with
# this program; if not, write to the Free Software Foundation, Inc., 59 Temple
# Place, Suite 330, Boston, MA  02111-1307  USA

EXPORT_FUNCTIONS()
{
    [[ -z "${ECLASS}" ]] && die "EXPORT_FUNCTIONS called but ECLASS undefined"

    local e
    for e in "$@" ; do
        case "$e" in
            pkg_nofetch|pkg_setup|pkg_prerm|pkg_postrm|pkg_preinst|pkg_postinst|pkg_config)
                eval "${e}() { ${ECLASS}_${e} \"\$@\" ; }"
                ;;

            src_unpack|src_compile|src_install|src_test)
                eval "${e}() { ${ECLASS}_${e} \"\$@\" ; }"
                ;;

            *)
                eval "${e}() { ${ECLASS}_${e} \"\$@\" ; }"
                ebuild_notice "qa" "$e should not be in EXPORT_FUNCTIONS for ${ECLASS}"
                ;;
        esac
    done
}

inherit()
{
    [[ -n "${PALUDIS_SKIP_INHERIT}" ]] && return

    local e ee location= v
    for e in "$@" ; do
        for ee in ${ECLASSDIRS:-${ECLASSDIR}} ; do
            [[ -f "${ee}/${e}.eclass" ]] && location="${ee}/${e}.eclass"
        done
        local old_ECLASS="${ECLASS}"
        export ECLASS="${e}"

        for v in ${PALUDIS_SOURCE_MERGED_VARIABLES} ; do
            local c_v="current_${v}" u_v="unset_${v}"
            local ${c_v}="${!v}"
            local ${u_v}="${!v-unset}"
            unset ${v}
        done

        [[ -z "${location}" ]] && die "Error finding eclass ${e}"
        source "${location}" || die "Error sourcing eclass ${e}"
        hasq "${ECLASS}" ${INHERITED} || export INHERITED="${INHERITED} ${ECLASS}"

        for v in ${PALUDIS_SOURCE_MERGED_VARIABLES} ; do
            local e_v="E_${v}"
            export ${e_v}="${!e_v} ${!v}"
        done

        for v in ${PALUDIS_SOURCE_MERGED_VARIABLES} ; do
            local c_v="current_${v}" u_v="unset_${v}"
            [[ "unset" == ${!u_v} ]] && unset ${v} || export ${v}="${!c_v}"
        done

        export ECLASS="${old_ECLASS}"
    done
}
