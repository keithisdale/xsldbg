%define name xsldbg
%define version 4.8.1
%define release 1
%define prefix /usr
%define builddir $RPM_BUILD_DIR/%{name}-%{version}
%define fileList %{builddir}/filelist

Summary: XSLT debugging/execution at console
Name: %{name}
Version: %{version}
Release: %{release}
Prefix: %{prefix}
Group: Development/Debuggers
License: GPLv2+
Distribution: Any GCC 3.2 based
Vendor: Keith Isdale <keithisdale@gmail.com>
Packager: Keith Isdale <keithisdale@gmail.com>
Source: %{name}-%{version}.tar.gz
URL: http://xsldbg.sourceforge.net/
BuildRequires: libQt5Core-devel, libxslt-devel, libxml2-devel
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

%description
A console tool to debug stylesheets(the eXtensible Stylesheet Language)
that has commands similar to the Unix/Linux debugger gdb.
It has three major modes of execution of stylesheets.
   o Run the whole stylesheet
   o Step to the next XSLT instruction
   o Continue until next break point is found, or the stylesheet has restarted

%prep
rm -rf $RPM_BUILD_ROOT
rm -rf %{builddir}

%setup -q

%build
echo "File list is %{fileList}"
echo "%{prefix}/bin/xsldbg" > %{fileList}
echo "%{prefix}/share/man/man1/xsldbg.1.gz" >> %{fileList}
echo "%{prefix}/share/doc/xsldbg" >> %{fileList}
QMAKE_EXTRAS=
if [ -d "%{prefix}/share/icons/hicolor" ]; then
   echo "Packaging xsldbg destktop shortcuts"
   QMAKE_EXTRAS="CONFIG+=xsldbg_shortcut"
   echo "%{prefix}/share/icons/hicolor/22x22/apps/xsldbg_source.png" >> %{fileList}
   echo "%{prefix}/share/icons/hicolor/128x128/apps/xsldbg_source.png" >> %{fileList}
   echo "%{prefix}/share/icons/hicolor/256x256/apps/xsldbg_source.png" >> %{fileList}
   echo "%{prefix}/share/icons/hicolor/512x512/apps/xsldbg_source.png" >> %{fileList}
   echo "%{prefix}/share/applications/xsldbg.desktop" >> %{fileList} 
fi

if [ -d "%{prefix}/share/doc/HTML/en" ]; then
   echo "Packaging KDE style documentation"
   QMAKE_EXTRAS="${QMAKE_EXTRAS} CONFIG+=xsdbg_kde_docs"
   echo "%{prefix}/share/doc/HTML/en/xsldbg" >> %{fileList}
fi

qmake-qt5 ${QMAKE_EXTRAS} -r xsldbg.pro
make

%install
rm -rf $RPM_BUILD_ROOT
INSTALL_ROOT=$RPM_BUILD_ROOT make install

cd $RPM_BUILD_ROOT
# compress the xsldbg manpage before file list is generated
gzip ./%{prefix}/share/man/man1/xsldbg.1

#echo "Sleeping for a minute to ensure that the timestamp in index.cashe.bz2 is correct"
#sleep 60
#touch ./%{kdeprefix}/share/doc/HTML/en/xsldbg/index.cache.bz2

%clean
rm -rf $RPM_BUILD_ROOT
rm -rf %{builddir}

%files -f filelist
%doc COPYING

%changelog
* Sat Oct 27 2018 Keith Isdale <keithisdale@gmail.com> - 4.8.1-1
- Build fixes for PLD-Linux included moving docs to /usr/share/docs/
* Sat Sep 08 2018 Keith Isdale <keithisdale@gmail.com> - 4.7.0-1
- Initial RPM release

