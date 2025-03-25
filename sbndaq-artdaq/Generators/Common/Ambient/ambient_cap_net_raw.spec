Name:		ambient_cap_net_raw
Version:	0.1
Release:	1%{?dist}

Summary:	A wrapper that runs a command with ambient cap_net_raw

License:	GPLv3+
URL:		http://servicedesk.fnal.gov

Source0:	ambient_exec-2.c

# Just leave the debug symbols in the binary
%define debug_package %{nil}

BuildRequires:	libcap-ng-devel gcc
Requires:	util-linux coreutils

%description
A wrapper binary that runs a command with the ambient capibility of
cap_net_raw.

%prep
echo 'nothing to do here'

%build
%{__cp} %{SOURCE0} ambient_cap_net_raw.c
gcc -Wall -lcap-ng ambient_cap_net_raw.c

%install
rm -rf %{buildroot}
%{__mkdir_p} %{buildroot}%{_libexecdir}
%{__install} -pm 755 a.out %{buildroot}%{_libexecdir}/ambient_cap_net_raw

%files
%defattr(0644,root,root,0755)
# If you can edit the memory this allocates, you can redirect the caps
#  but since this lets you run any command anyway... no extra harm here
%attr(0755,root,root) %caps(cap_net_raw=p) %{_libexecdir}/ambient_cap_net_raw

%changelog
* Tue Mar 17 2020 Ron Rechenmacher <ron@fnal.gov> - 0.1-1
- Initial Build for RITM0946313
