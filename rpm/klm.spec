Name:           klm
Version:		0.1.9
Release:        1%{?dist}
Summary:        OpenKJ karaoke library manager

License:        GPL
URL:            https://openkj.org
Source0:		klm-0.1.9-ALPHA.5.tar.gz

BuildRequires:  cmake qt5-qtbase-devel zlib-devel git
Requires:       qt5-qtbase zlib

%description
Tool to help manage large karaoke libraries (mp3g, mp3g-zip, and video files)

%prep
%setup

%build
git submodule update --init
%cmake -DCMAKE_BUILD_TYPE=Release
%cmake_build

%install
%cmake_install

%files
/usr/bin/klm
/usr/share/applications/klm.desktop
/usr/share/pixmaps/klm.svg

%changelog
* Wed May 5 2021 T. Isaac Lightburn <isaac@openkj.org>
- 
