Name:           unikey-wayland
Version:        1.0.0
Release:        1%{?dist}
Summary:        Unikey Wayland Input Method for Vietnamese

License:        GPL
URL:            https://github.com/truonghieu/unikey-wayland

# Disable debuginfo package generation
%define debug_package %{nil}

%description
Unikey-Wayland is a lightweight Vietnamese input method for Wayland environments, powered by the UniKey engine and Qt 6 GUI.

%prep
# Nothing to prepare since we are packaging precompiled binaries

%build
# Nothing to build since we are packaging precompiled binaries

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/usr/bin
mkdir -p %{buildroot}/usr/share/applications

# Copy our pre-compiled files from the SOURCES directory
cp %{_sourcedir}/unikey-wayland %{buildroot}/usr/bin/unikey-wayland
cp %{_sourcedir}/unikey-wayland.desktop %{buildroot}/usr/share/applications/unikey-wayland.desktop

# Ensure correct permissions
chmod 755 %{buildroot}/usr/bin/unikey-wayland
chmod 644 %{buildroot}/usr/share/applications/unikey-wayland.desktop

%files
/usr/bin/unikey-wayland
/usr/share/applications/unikey-wayland.desktop

%changelog
* Tue Jul 07 2026 truonghieu - 1.0.0-1
- Initial release under the name unikey-wayland
