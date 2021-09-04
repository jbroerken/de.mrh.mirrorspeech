# de.mrh.mirrorspeech

<p align="center">
<img width="100%" height="100%" src="/doc/source/banner.png">
</p>


## About

The de.mrh.mirrorspeech project implements a simple "Repeat after me" application.


## Requirements

#### Compilation

This project is built using CMake. You can find CMake here:

https://cmake.org/

#### Library Dependencies

This project requires other libraries and headers to function:

Dependency | Source
---------- | ------
libmrh |  https://github.com/jbroerken/libmrh/
libmrhab | https://github.com/jbroerken/libmrhab/
libmrhbf |  https://github.com/jbroerken/libmrhbf/
libmrhcevs | https://github.com/jbroerken/libmrhcevs/
libmrhvt | https://github.com/jbroerken/libmrhvt/
mrhshared | https://github.com/jbroerken/mrhshared/

For more information about the requirements, check the "Building" section found in the documentation.


## Documentation

All documentation is build with sphinx-doc using the Read The Docs theme.
To build the documentation, grab the requirements for it:

#### sphinx-doc
https://www.sphinx-doc.org/en/master/

#### Read The Docs Theme
https://sphinx-rtd-theme.readthedocs.io/en/stable/

## Licence

This project is licenced under the GNU General Public 2 licence. 
Please read the included LICENCE.txt for the exact terms.


## Directories

This project supplies multiple directories for the development of said project. 
Their names and descriptions are as follows:

Directory | Description
--------- | -----------
bin | Contains the built project executables.
build | CMake build directory.
doc | Documentation files.
pkg | The project package directory (files, configuration, etc.).
src | Project source code.
