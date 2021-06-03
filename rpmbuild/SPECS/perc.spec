Summary:        Tool for scanning directories for accessible files
Name:           perc
Version:        0.1.0
Release:        0
License:        GPL
URL:            https://github.com/elatypov20/perc
Group:          File tools
Source0:        %{name}-%{version}.tar.bz2
BuildArch:      armv7hl

%description
perc is a tool for recursively detecting to which files user and group can write in some path.

%prep
%setup

%make

%install
cp bin/perc /usr/bin/perc