#!/bin/bash

revision=
if test -d ".svn"; then

  revision=`cd "$1" && LC_ALL=C svn info 2> /dev/null | grep Revision | cut -d' ' -f2`
  test $revision || revision=`cd "$1" && grep revision .svn/entries 2>/dev/null | cut -d '"' -f2`
  test $revision || revision=`cd "$1" && sed -n -e '/^dir$/{n;p;q}' .svn/entries 2>/dev/null`
  # The revision is unknown
  test $revision || revision="0"
  revision="0.$revision"

elif test -d ".git"; then

  # If taged git releases are eventually used:
  # Git has no logical revision structure so tag releases
  # revision=`git describe 2> /dev/null`

  # currently git is used for development, therefore a version number of zero will do fine
  # you should not be giving this to anyone and therefore a version number of 0.0 will let them
  # know that it is completely unstable
  test $revision || revision=`git describe --tags HEAD | sed 's/^v//' | sed 's/-[^-]*$//'`
  test $revision || revision="0.0"

else
  # this check is here for the tar pacakge which will be of the form 'fuppes-version'
  wd=`pwd`
  dir=`basename "$wd"`
  if [[ $dir =~ fuppes\-0\.[0-9]+ ]]
  then
    revision=`pwd | sed 's_\/_\n_g' | tail -n1 | cut -d- -f2`
  else
    # The default revision number, manually change to whatever you want if you are not currently
    # using a scm tool.
    revision="0.655"
  fi
fi

printf "$revision"
