# Contributing to mondevtopromisc

Thanks for wanting to contribute! It's really appreciated.

The following is a set of guidelines for contributing to mondevtopromisc.

## Code of Conduct

This project and everyone participating in it is governed by the [Code of Conduct](CODE_OF_CONDUCT.md). By participating, you are expected to uphold this code. Please report unacceptable behavior to [rick.04.1996@gmail.com](mailto:rick.04.1996@gmail.com).

## What should I know before I get started?

### Packages

The packages you need are essentially the same as in the build guide of the [Read Me](README.md) with the addition of the following programs/packages.

- clang-format-11
- clang-tidy-11
- doxygen

These packages add static code analysis, code formatting and code documentation.
It is important that you use these packages when contributing by running the formatter over your code, checking for warning with clang-tidy and checking if the code documentation is still properly generated and new code is properly documented.

### Design Decisions

Design decisions are at the discretion of [codedwrench](https://github.com/codedwrench), feel free to suggest changes to the code's structure to make it better. Code may be rejected if it doesn't conform to the current style.

## How Can I Contribute?

### Reporting Bugs

If you find anything that does not work, it's important to file a bug report. When you do file a bug report the following information is very important for this project.

- What WiFi adapter were you using?
- What version of XLink Kai was running.
- Could the device be seen in XLink Kai?
- What commit of mondevtopromisc did the bug happen in?
- How do you reproduce the bug? Or are you not able to reproduce it easily?

Other useful information to supplement are the following things:
- The log.txt that mondevtopromisc produces with mondevtopromisc preferably in TRACE mode.
- A capture file (wireshark) of when the bug occured (this may show some private information about your network, so only do this if you're okay with that, mailing this file to me personally is allowed.)
- If on linux, the dmesg file outputted to a file.

Label bugs as "bug" and try to add labels that seem appropriate.

### Suggesting Enhancements

Feel free to submit enhancements as an issue, but do tag the issue as an "enhancement" and check if none of the other issues opened or closed already suggested it.
Also try to attach labels that seem to match with the enhancement, like "backend".

### Your First Code Contribution

Feel free to pick up any of the unassigned work on the issues page, some issues are marked as 'good first issue', these should be fairly easy to pick up.
Once you're done making your code contributions, make a pull request with your code and it will be reviewed.

## Styleguides

### Git Commit Messages

Try to make the commit messages as clear as possible without going past 72 characters. Preferably start with a verb like: add, move, change.

### Documentation Styleguide

There is no real styleguide here yet.
Accepted formats: md, latex, doc.
Doc files are not preferred, docx files are not allowed.

### C++ Styleguide

This project uses CamelCase for function names, variables and class names.

When variables are initialized use the brace initializer as much as possible.

Enum variables are in all caps.

Prefix member variables with: m \
Prefix local variables with: l \
Prefix global variables with: g \
Prefix constants with: c

Constants are put in their own namespace, named ClassName_Constants.

Use clang formatter to format the code. \
Try to conform to the rest of the code. \
Try to write modern testable c++ code.

