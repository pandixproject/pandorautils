  PANDORAUTILS
A SOFTWARE BY THE PANDIX PROJECT

Pandorautils is a small, simple remake (or demake) of selected GNU
coreutils programs. It is a work in progress by the Pandix Project.

  PROGRAMS
    pcat     concatenate files and print them to standard output
    pbasename print a file name with directory components removed
    pchmod   change file permissions
    pcp      copy files and directories
    pdate    display the current date and time
    pdirname print a path with its last component removed
    pecho    print text
    phead    print the first lines of files
    plink    create hard or symbolic links
    pls      list directory contents
    pmkdir   create directories
    pmv      move and rename files
    ppwd     print the current working directory
    preadlink print the value of a symbolic link
    prm      remove files and directories
    prmdir   remove empty directories
    psort    sort lines of text
    ptail    print the last lines of files
    ptee     copy standard input to files and standard output
    ptouch   create files and update timestamps
    pwhoami  print the current user
    pwc      count lines, words, and bytes
    pyes     repeatedly print a string
    puniq    report or omit repeated lines

  COMPILING
A C compiler and make are required. Build every program with:
    make

The binaries are written to the bin/ directory. To remove build output:
    make clean

Run the regression suite with:
    make test

  RUNNING
Run a program from the bin/ directory, for example:
    ./bin/pcat README.txt

  INSTALLING
By default, the programs are installed to /usr/local/bin:
    sudo make install

Choose another prefix when needed:
    make install PREFIX=$HOME/.local

Package builders can stage installation with DESTDIR:
    make install DESTDIR=/path/to/staging/root

To remove installed programs from the same prefix and destination directory:
    sudo make uninstall

  LICENSE
Pandorautils is dedicated to the public domain under the Creative
Commons Zero v1.0 Universal (CC0 1.0) license. See LICENSE.
