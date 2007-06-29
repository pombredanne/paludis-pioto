/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_GENTOO_E_KEY_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_GENTOO_E_KEY_HH 1

#include <paludis/metadata_key.hh>
#include <paludis/util/fs_entry-fwd.hh>

namespace paludis
{
    class PackageID;

    namespace erepository
    {
        class EStringKey :
            public MetadataStringKey
        {
            private:
                const std::string _value;

            public:
                EStringKey(const tr1::shared_ptr<const PackageID> &,
                        const std::string &, const std::string &, const std::string &, const MetadataKeyType);
                ~EStringKey();

                virtual const std::string value() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class EDependenciesKey :
            public MetadataSpecTreeKey<DependencySpecTree>,
            private PrivateImplementationPattern<EDependenciesKey>
        {
            private:
                Implementation<EDependenciesKey> * const _imp;

            public:
                EDependenciesKey(const tr1::shared_ptr<const PackageID> &,
                        const std::string &, const std::string &, const std::string &, const MetadataKeyType);
                ~EDependenciesKey();

                virtual const tr1::shared_ptr<const DependencySpecTree::ConstItem> value() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class EURIKey :
            public MetadataSpecTreeKey<URISpecTree>,
            private PrivateImplementationPattern<EURIKey>
        {
            private:
                Implementation<EURIKey> * const _imp;

            public:
                EURIKey(const tr1::shared_ptr<const PackageID> &,
                        const std::string &, const std::string &, const std::string &, const MetadataKeyType);
                ~EURIKey();

                virtual const tr1::shared_ptr<const URISpecTree::ConstItem> value() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class ERestrictKey :
            public MetadataSpecTreeKey<RestrictSpecTree>,
            private PrivateImplementationPattern<ERestrictKey>
        {
            private:
                Implementation<ERestrictKey> * const _imp;

            public:
                ERestrictKey(const tr1::shared_ptr<const PackageID> &,
                        const std::string &, const std::string &, const std::string &, const MetadataKeyType);
                ~ERestrictKey();

                virtual const tr1::shared_ptr<const RestrictSpecTree::ConstItem> value() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class EProvideKey :
            public MetadataSpecTreeKey<ProvideSpecTree>,
            private PrivateImplementationPattern<EProvideKey>
        {
            private:
                Implementation<EProvideKey> * const _imp;

            public:
                EProvideKey(const tr1::shared_ptr<const PackageID> &,
                        const std::string &, const std::string &, const std::string &, const MetadataKeyType);
                ~EProvideKey();

                virtual const tr1::shared_ptr<const ProvideSpecTree::ConstItem> value() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class ELicenseKey :
            public MetadataSpecTreeKey<LicenseSpecTree>,
            private PrivateImplementationPattern<ELicenseKey>
        {
            private:
                Implementation<ELicenseKey> * const _imp;

            public:
                ELicenseKey(const tr1::shared_ptr<const PackageID> &,
                        const std::string &, const std::string &, const std::string &, const MetadataKeyType);
                ~ELicenseKey();

                virtual const tr1::shared_ptr<const LicenseSpecTree::ConstItem> value() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class EIUseKey :
            public MetadataCollectionKey<IUseFlagCollection>,
            private PrivateImplementationPattern<EIUseKey>
        {
            private:
                Implementation<EIUseKey> * const _imp;

            public:
                EIUseKey(const tr1::shared_ptr<const PackageID> &,
                        const std::string &, const std::string &, const std::string &, const MetadataKeyType);
                ~EIUseKey();

                const tr1::shared_ptr<const IUseFlagCollection> value() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class EKeywordsKey :
            public MetadataCollectionKey<KeywordNameCollection>,
            private PrivateImplementationPattern<EKeywordsKey>
        {
            private:
                Implementation<EKeywordsKey> * const _imp;

            public:
                EKeywordsKey(const tr1::shared_ptr<const PackageID> &,
                        const std::string &, const std::string &, const std::string &, const MetadataKeyType);
                ~EKeywordsKey();

                const tr1::shared_ptr<const KeywordNameCollection> value() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class EUseKey :
            public MetadataCollectionKey<UseFlagNameCollection>,
            private PrivateImplementationPattern<EUseKey>
        {
            private:
                Implementation<EUseKey> * const _imp;

            public:
                EUseKey(const tr1::shared_ptr<const PackageID> &,
                        const std::string &, const std::string &, const std::string &, const MetadataKeyType);
                ~EUseKey();

                const tr1::shared_ptr<const UseFlagNameCollection> value() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class EInheritedKey :
            public MetadataCollectionKey<InheritedCollection>,
            private PrivateImplementationPattern<EInheritedKey>
        {
            private:
                Implementation<EInheritedKey> * const _imp;

            public:
                EInheritedKey(const tr1::shared_ptr<const PackageID> &,
                        const std::string &, const std::string &, const std::string &, const MetadataKeyType);
                ~EInheritedKey();

                const tr1::shared_ptr<const InheritedCollection> value() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class EContentsKey :
            public MetadataContentsKey,
            private PrivateImplementationPattern<EContentsKey>
        {
            private:
                Implementation<EContentsKey> * const _imp;

            public:
                EContentsKey(const tr1::shared_ptr<const PackageID> &,
                        const std::string &, const std::string &, const FSEntry &, const MetadataKeyType);
                ~EContentsKey();

                const tr1::shared_ptr<const Contents> value() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class ECTimeKey :
            public MetadataTimeKey,
            private PrivateImplementationPattern<ECTimeKey>
        {
            private:
                Implementation<ECTimeKey> * const _imp;

            public:
                ECTimeKey(const tr1::shared_ptr<const PackageID> &,
                        const std::string &, const std::string &, const FSEntry &, const MetadataKeyType);
                ~ECTimeKey();

                const time_t value() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));
        };
    }
}

#endif