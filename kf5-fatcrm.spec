Name:           kf5-fatcrm
Version:        2.1.0
Release:        0
Summary:        desktop application for SugarCRM
Source:         %{name}-%{version}.tar.gz
Url:            https://github.com/KDAB/FatCRM
Group:          Productivity/Other
License:        GPL-2.0+
BuildRoot:      %{_tmppath}/%{name}-%{version}-build
Vendor:         Klaralvdalens Datakonsult AB (KDAB)
Packager:       Klaralvdalens Datakonsult AB (KDAB) <info@kdab.com>

BuildRequires: qt5-kdsoap-devel qt5-kdreports-devel
%if %{defined suse_version}
BuildRequires:  python libqt5-qtbase-devel cmake update-desktop-files extra-cmake-modules kdbusaddons-devel akonadi-server-devel kcalcore-devel kcontacts-devel akonadi-contact-devel akonadi-calendar-devel kwallet-devel libicu-devel
%endif

%if %{defined fedora}
BuildRequires:  gcc-c++ extra-cmake-modules shared-mime-info python qt5-qtbase-devel cmake desktop-file-utils kf5-kwallet-devel kf5-kcontacts-devel kf5-akonadi-server-devel kf5-kcalendarcore-devel kf5-akonadi-contacts-devel
%endif

%description
FatCRM is a desktop application for SugarCRM.

SugarCRM (and its opensource version SuiteCRM) is a web-oriented system for tracking
customers and potential customers. The downside of web user interfaces is the latency
that applies to every action, requiring roundtrips to the web server.

FatCRM solves this by creating a local cache of the data (accounts, opportunities,
contacts, etc.) and providing a desktop application to search, view, and edit this data.

Authors:
--------
     Klaralvdalens Datakonsult AB (KDAB) <info@kdab.com>

%define debug_package %{nil}
%global __debug_install_post %{nil}

%prep
%setup -T -c
%{__tar} -zxf %{SOURCE0} --strip-components=1

%build
cmake . -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release
%__make %{?_smp_mflags}

%install
%make_install

%if %{defined suse_version}
%suse_update_desktop_file -c org.kde.fatcrm FatCRM "Desktop application for SugarCRM" fatcrm fatcrm Utility DesktopUtility
%suse_update_desktop_file fatcrminvoker Utility DesktopUtility
%endif

%clean
%__rm -rf "%{buildroot}"

%files
%defattr(-,root,root)
%{_sysconfdir}/xdg/fatcrm.categories
%{_libdir}/libkdcrmdata.*
%{_libdir}/libfatcrmprivate.*
%if %{defined suse_version}
%dir %{_prefix}/share/akonadi/plugins/
%dir %{_prefix}/share/akonadi/plugins/serializer
%{_libdir}/qt5/plugins/akonadi_serializer_*
%else
%if 0%{?fedora} > 28
%{_libdir}/qt5/plugins/akonadi_serializer_*
%else
%{_libdir}/plugins/akonadi_serializer_*
%endif
%endif
%{_prefix}/share/akonadi/plugins/serializer/akonadi_serializer_*.desktop
%{_prefix}/share/mime/packages/kdabcrm-mime.xml
%{_prefix}/bin/fatcrm
%{_prefix}/bin/fatcrminvoker
%{_prefix}/bin/akonadi_salesforce_resource
%{_prefix}/bin/akonadi_sugarcrm_resource
%{_prefix}/share/applications/org.kde.fatcrm.desktop
%{_prefix}/share/applications/fatcrminvoker.desktop
%{_prefix}/share/icons/hicolor
%dir %{_prefix}/share/akonadi/
%dir %{_prefix}/share/akonadi/agents/
%{_prefix}/share/akonadi/agents/salesforceresource.desktop
%{_prefix}/share/akonadi/agents/sugarcrmresource.desktop

%changelog
* Sun Jun 24 2018 Allen Winter <allen.winter@kdab.com> 2.1.0
- 2.1.0 release
* Wed Nov 09 2016 Allen Winter <allen.winter@kdab.com> 1.3.2
- 1.3.2 pre-release
