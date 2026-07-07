Name:           uk-wayland-im
Version:        1.0.0
Release:        1%{?dist}
Summary:        UK Wayland Input Method for Vietnamese

License:        GPL
URL:            https://github.com/truonghieu/Uk362

# Disable debuginfo package generation
%define debug_package %{nil}

%description
UK Wayland Input Method is a lightweight Vietnamese input method for Wayland environments, powered by the UniKey engine.

%prep
# Nothing to prepare since we are packaging precompiled binaries

%build
# Nothing to build since we are packaging precompiled binaries

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/usr/bin
mkdir -p %{buildroot}/usr/share/applications

# Copy our pre-compiled files from the SOURCES directory
cp %{_sourcedir}/uk-wayland-im %{buildroot}/usr/bin/uk-wayland-im
cp %{_sourcedir}/uk-wayland-im.desktop %{buildroot}/usr/share/applications/uk-wayland-im.desktop

# Ensure correct permissions
chmod 755 %{buildroot}/usr/bin/uk-wayland-im
chmod 644 %{buildroot}/usr/share/applications/uk-wayland-im.desktop

%files
/usr/bin/uk-wayland-im
/usr/share/applications/uk-wayland-im.desktop

%changelog
* Tue Jul 07 2026 truonghieu - 1.0.0-1
- Initial release
