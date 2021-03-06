#
# Conditional build:
%bcond_without	kde		# install KDE Docbook documentation

# some macros that set for all distro, TODO make use of convensions in https://en.opensuse.org/openSUSE:Packaging_Conventions_RPM_Macros
%define prefix /usr
%define _iconsdir %{prefix}/share/icons
%define _desktopdir %{prefix}/share/applications
%define _kdedocdir %{prefix}/share/doc/HTML

Summary:	XSLT stylesheets debugger
Summary(pl.UTF-8):	Odpluskiwacz styli XSLT
Name:		xsldbg
Version:	4.8.1
Release:	1
License:	GPL
Group:		Development/Debuggers
Source0:	http://downloads.sourceforge.net/xsldbg/%{name}-%{version}.tar.gz
# Source0-md5:	e9bf58a2f81c19279ddc9d686a464902
URL:		http://xsldbg.sourceforge.net/
%{?with_kde:BuildRequires: kdoctools}
BuildRequires:	libstdc++-devel
BuildRequires:	libxml2-devel
BuildRequires:	libxslt-devel
BuildRequires:	perl-base
BuildRequires:	readline-devel
Requires:	desktop-file-utils
Requires:	hicolor-icon-theme
BuildRoot:	%{tmpdir}/%{name}-%{version}-root-%(id -u -n)

%description
xsldbg is debugger with interface similar to gdb, but used to debug
XSLT stylesheets.

It has three major modes of execution of stylesheets: run the whole
stylesheet; step to next xsl instruction; continue until next break
point is found, or stylesheet has restarted.

%description -l pl.UTF-8
xsldbg jest odpluskwiaczem z interfejsem podobnym do gdb, ale służącym
do odpluskwiania styli XSLT.

Ma trzy podstawowe tryby wykonywania styli: uruchomienie całości; krok
do następnej instrukcji xsl; kontynuacja do następnego punktu stopu
lub restartu stylu.

%package apidocs
Summary:	xsldbg KDE Docbook
Group:		Documentation
BuildArch:	noarch

%description apidocs
xsldbg KDE Docbook.

%prep
%setup -q

%build
qmake-qt5 \
	"CONFIG+=xsldbg_shortcut" \
	ICONS_DIR="%{_iconsdir}" \
	DOCS_ROOT="%{_docdir}/xsldbg" \
	KDEDOCS_ROOT="%{_kdedocdir}" \
	-r xsldbg.pro
%{__make}

%install
rm -rf $RPM_BUILD_ROOT
%{__make} install \
	INSTALL_ROOT=$RPM_BUILD_ROOT

%clean
rm -rf $RPM_BUILD_ROOT

%post
%update_desktop_database
%update_icon_cache hicolor

%postun
%update_desktop_database
%update_icon_cache hicolor

%files
%defattr(644,root,root,755)
%doc README.md AUTHORS ChangeLog TODO

%attr(755,root,root) %{_bindir}/xsldbg
%{_mandir}/man1/xsldbg.1*
%if %{with kde}
%{_desktopdir}/xsldbg.desktop
%{_iconsdir}/hicolor/*/apps/xsldbg_source.png
%endif

%if %{with kde}
%files apidocs
%defattr(644,root,root,755)
%{_kdedocdir}/en/xsldbg/index.cache.bz2
%{_kdedocdir}/en/xsldbg/index.docbook
%endif

%changelog
* Sun Oct 28 2018 Keith Isdale <keithisdale@gmail.com> - 4.8.1-1
- Replace spec file based on once created by PLD Linux contributor
- Build fixes for PLD-Linux included fixing install paths for docs and icons
* Sat Sep 08 2018 Keith Isdale <keithisdale@gmail.com> - 4.7.0-1
- Initial RPM release
