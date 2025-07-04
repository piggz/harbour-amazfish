# Define Sailfish as it is absent
%if !0%{?fedora}
%define sailfishos 1
%endif

# Prevent brp-python-bytecompile from running.
%define __os_install_post %{___build_post}

%if 0%{?sailfishos}
# "Harbour RPM packages should not provide anything."
%define __provides_exclude_from ^%{_datadir}/.*$
%endif

Name:       harbour-amazfish

# >> macros
# << macros

%{!?qtc_qmake:%define qtc_qmake %qmake}
%{!?qtc_qmake5:%define qtc_qmake5 %qmake5}
%{!?qtc_make:%define qtc_make make}
%{?qtc_builddir:%define _builddir %qtc_builddir}

Summary:    Watch interface application
Version:    2.7.0
Release:    1
Group:      QtQt
License:    GPLv3
URL:        https://piggz.co.uk
Source0:    %{name}-%{version}.tar.bz2
Requires:   sailfishsilica-qt5 >= 0.10.9
Requires:   kdb-sqlite3-driver >= 3.1.0
Requires:   libKDb3-3 >= 3.1.0
Requires:   kcoreaddons >= 5.31.0
Requires:   libicu
Requires:   kf5bluezqt
Requires:   libkf5archive
Requires:   mapboxgl-qml
BuildRequires:  pkgconfig(sailfishapp) >= 1.0.2
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5Qml)
BuildRequires:  pkgconfig(Qt5Quick)
BuildRequires:  pkgconfig(Qt5Contacts)
BuildRequires:  pkgconfig(Qt5Widgets)
BuildRequires:  pkgconfig(Qt5Xml)
BuildRequires:  pkgconfig(Qt5Positioning)
BuildRequires:  pkgconfig(mlite5)
BuildRequires:  pkgconfig(libmkcal-qt5)
BuildRequires:  (kf5-calendarcore-devel if kf5-calendarcore)
BuildRequires:  pkgconfig(zlib)
BuildRequires:  pkgconfig(mpris-qt5)
BuildRequires:  kdb-devel >= 3.1.0
BuildRequires:  kcoreaddons-devel >= 5.31.0
BuildRequires:  qt5-qttools-linguist
BuildRequires:  desktop-file-utils
BuildRequires:  libkeepalive-devel
BuildRequires:  libkf5archive-devel
BuildRequires:  pulseaudio-devel
BuildRequires:  libicu-devel
BuildRequires:  cmake
BuildRequires:  ninja

%description
Watch companion application for SalfishOS

%if 0%{?_chum}
Title: Amazfish
Type: desktop-application
DeveloperName: Adam Pigg
Categories:
 - Utility
Custom:
  Repo: https://github.com/piggz/harbour-amazfish
PackageIcon: https://raw.githubusercontent.com/piggz/harbour-amazfish/master/ui/icons/scalable/harbour-amazfish-ui.svg
Screenshots:
 - https://github.com/piggz/harbour-amazfish/raw/master/screenshots/screenshot_main.png
 - https://github.com/piggz/harbour-amazfish/raw/master/screenshots/screenshot_activity.png
 - https://github.com/piggz/harbour-amazfish/raw/master/screenshots/screenshot_sleep.png
Links:
  Homepage: https://github.com/piggz/harbour-amazfish
  Help: https://github.com/piggz/harbour-amazfish/discussions
  Bugtracker: https://github.com/piggz/harbour-amazfish/issues
  Donation: https://www.paypal.me/piggz
%endif

%prep
%setup -q -n %{name}-%{version}

# >> setup
# << setup

%build
%if 0%{?sailfishos}
    cmake \
        -DCMAKE_INSTALL_PREFIX:PATH='/usr' \
        -DFLAVOR=silica \
        .
%else
    cmake \
        -DCMAKE_INSTALL_PREFIX:PATH='/usr' \
        -DFLAVOR=kirigami \
        .
%endif

cmake --build "." %{?_smp_mflags} --verbose

# >> build post
# << build post

%install
rm -rf %{buildroot}
# >> install pre
# << install pre

DESTDIR=%{buildroot} cmake --install .

# >> install post
# << install post

desktop-file-install --delete-original       \
  --dir %{buildroot}%{_datadir}/applications             \
   %{buildroot}%{_datadir}/applications/*.desktop

rm -rf %{buildroot}/usr/include/o2
rm -f %{buildroot}/usr/lib/libo2.a
rm -f %{buildroot}/usr/share/metainfo/harbour-amazfish-ui.appdata.xml

%files
%defattr(-,root,root,-)
%{_bindir}/%{name}-ui
%{_bindir}/%{name}d
%attr(2755,root,privileged) %{_bindir}/%{name}d
%{_datadir}/%{name}d
%{_datadir}/%{name}-ui
%{_datadir}/applications/%{name}-ui.desktop
%{_datadir}/icons/hicolor/*/apps/%{name}-ui.png
%{_datadir}/icons/hicolor/scalable/apps/%{name}-ui.svg
%{_userunitdir}/harbour-amazfish.service
%{_datadir}/mapplauncherd/privileges.d/harbour-amazfishd.privileges
%{_datadir}/harbour-amazfishd/chirp.raw
# >> files
# << files
