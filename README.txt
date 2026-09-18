  PANDORAUTILS
A SOFTWARE BY THE PANDIX PROJECT

Pandorautils is a small, simple remake (or demake) of selected GNU
coreutils programs. It is a work in progress by the Pandix Project.

  PROGRAMS
    cat      concatenate files and print them to standard output
    basename  print a file name with directory components removed
    chmod    change file permissions
    cp       copy files and directories
    date     display the current date and time
    dirname  print a path with its last component removed
    echo     print text
    head     print the first lines of files
    link     create hard or symbolic links
    ls       list directory contents
    mkdir    create directories
    mv       move and rename files
    pwd      print the current working directory
    readlink  print the value of a symbolic link
    rm       remove files and directories
    rmdir    remove empty directories
    sort     sort lines of text
    tail     print the last lines of files
    tee      copy standard input to files and standard output
    touch    create files and update timestamps
    whoami   print the current user
    wc       count lines, words, and bytes
    yes      repeatedly print a string
    uniq     report or omit repeated lines
    stat     display file or filesystem status
    find     search for files in a directory hierarchy
    file     determine file type
    true     returns true
    false    returns false
    which    locate a command
    sleep    suspend execution for a specified amount of time
    grep     search for patterns in text
    pgrep    search for processes by name

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
