# Kabanero

> Powered by sausage 🌭 and the hot stuff 🌶

[![build status](https://gitlab.com/mureke/kabanero/badges/master/build.svg)](https://gitlab.com/mureke/kabanero/commits/master)
[![coverage report](https://gitlab.com/mureke/kabanero/badges/master/coverage.svg)](https://gitlab.com/mureke/kabanero/commits/master)

## Dependencies

### Lua

Install Lua with

```
brew install lua
```

or your environment's equivalent.
Version >=5.2.4 is required.

### SFML

Install [SFML](http://www.sfml-dev.org/) with

```
brew install sfml
```

or your environment's equivalent.
Version >=2.2.0 is required.

### CMake

Install [CMake](https://cmake.org/)

## Installing

Clone the project:

```
git clone https://gitlab.com/mureke/kabanero.git
```

Pull and update all submodules:

```
git pull && git submodule update --init --recursive
```

Build assets:

```
./build.sh assets
```

Build and run:

```
./build.sh run
```

## Tools

To build the project, you can use `build.sh` script.

*   To make use argument: `./build.sh make`
*   To run after building: `./build.sh run`
*   To run tests: `./build.sh test`
*   To clean up: `./build.sh clean`
*   To create documentation: `./build.sh docs`

This project is developed and tested with clang compiler and c++14 features. There are some known issues with GNU compiler and it is not supported.

## Development environment

Please use [atom](https://atom.io/).

Some useful packages for making development more enjoyable:

*   [autocomplete-clang](https://atom.io/packages/autocomplete-clang)
*   [autocomplete-emojis](https://atom.io/packages/autocomplete-emojis)
*   [build](https://atom.io/packages/build)
*   [linter-clang](https://atom.io/packages/linter-clang)

## Documentation

You can build the documentation with command:

```
./build.sh docs
```
Documentation will be created to path `build/html`. Access them by opening `index.html` in your [favorite html rendering program](https://www.google.com/chrome/browser/desktop/index.html).

Note that building documentation requires [Doxygen](http://www.stack.nl/~dimitri/doxygen/).

## Contributing

### Git Hooks

Activate git hooks by running command `activate_hooks.sh` before committing.

### Message icons

Always start **commit messages** and **issue titles** with one of these emojis:

**Note** that you don't put the actual emoji in the message.
Please use the markdown emoji format eg. `:tada:` instead of 🎉

Commit Type             | Emoji
----------------------- | -------------------
Initial Commit          | 🎉  `:tada:`
Version Tag             | 🔖  `:bookmark:`
New Feature             | ✨  `:sparkles:`
Bugfix                  | 🐛  `:bug:`
Refactoring             | 📦  `:package:`
Documentation           | 📚  `:books:`
Performance             | 🐎  `:racehorse:`
Cosmetic                | 💄  `:lipstick:`
Tooling                 | 🔧  `:wrench:`
Tests                   | 🚨  `:rotating_light:`
Deprecation             | 💩  `:poop:`
Work In Progress (WIP)  | 🚧  `:construction:`
Library                 | 📓  `:notebook:`
Remove linter warnings  | 👕  `:shirt:`
Remove code/files       | 🔥  `:fire:`
Build process           | 👷  `:construction_worker:`
