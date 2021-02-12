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
Version:    1.9.6
Release:    1
Group:      QtQt
License:    LICENSE
URL:        http://example.org/
Source0:    %{name}-%{version}.tar.bz2
Requires:   sailfishsilica-qt5 >= 0.10.9
Requires:   kdb-sqlite3-driver >= 3.1.0
Requires:   libKDb3-3 >= 3.1.0
Requires:   kcoreaddons >= 5.31.0
Requires:   libicu
Requires:   qt5-qtconnectivity-qtbluetooth
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
BuildRequires:  pkgconfig(libkcalcoren-qt5)
BuildRequires:  pkgconfig(zlib)
BuildRequires:  pkgconfig(mpris-qt5)
BuildRequires:  kdb-devel >= 3.1.0
BuildRequires:  kcoreaddons-devel >= 5.31.0
BuildRequires:  qt5-qttools-linguist
BuildRequires:  qt5-qtconnectivity-qtbluetooth-devel
BuildRequires:  desktop-file-utils

%description
Short description of my Sailfish OS Application


%prep
%setup -q -n %{name}-%{version}

# >> setup
# << setup

%build
%if 0%{?sailfishos}
%qmake5 VERSION='%{version}-%{release}' FLAVOR=silica
%else
%qmake5 VERSION='%{version}-%{release}' FLAVOR=kirigami
%endif

%qtc_make %{?_smp_mflags}

# >> build post
# << build post

%install
rm -rf %{buildroot}
# >> install pre
# << install pre
%qmake5_install

# >> install post
# << install post

desktop-file-install --delete-original       \
  --dir %{buildroot}%{_datadir}/applications             \
   %{buildroot}%{_datadir}/applications/*.desktop

%files
%defattr(-,root,root,-)
%{_bindir}/%{name}-ui
%{_bindir}/%{name}d
%{_datadir}/%{name}-ui
%{_datadir}/applications/%{name}-ui.desktop
%{_datadir}/icons/hicolor/*/apps/%{name}-ui.png
%{_datadir}/icons/hicolor/scalable/apps/%{name}.svg
%{_libdir}/systemd/user/harbour-amazfish.service
%{_datadir}/mapplauncherd/privileges.d/harbour-amazfishd.privileges
# >> files
# << files
