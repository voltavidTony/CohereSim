#!/bin/bash

# Code copied and adapted from tools/parsec-benchmark/bin/parsecmgmt
#  so that it can be sourced without PARSEC attempting to evaluate a command

# Determine OS name to use for automatically determined PARSECPLAT
case "${OSTYPE}" in
  *linux*)   ostype="linux";;
  *solaris*) ostype="solaris";;
  *bsd*)     ostype="bsd";;
  *aix*)     ostype="aix";;
  *hpux*)    ostype="hpux";;
  *irix*)    ostype="irix";;
  *amigaos*) ostype="amigaos";;
  *beos*)    ostype="beos";;
  *bsdi*)    ostype="bsdi";;
  *cygwin*)  ostype="windows";;
  *darwin*)  ostype="darwin";;
  *interix*) ostype="interix";;
  *os2*)     ostype="os2";;
  *osf*)     ostype="osf";;
  *sunos*)   ostype="sunos";;
  *sysv*)    ostype="sysv";;
  *sco*)     ostype="sco";;
  *)         ostype="${OSTYPE}";;
esac

# Determine HOST name to use for automatically determined PARSECPLAT
case "${HOSTTYPE}" in
  *i386*)    hosttype="i386";;
  *x86_64*)  hosttype="amd64";;
  *amd64*)   hosttype="amd64";;
  *i486*)    hosttype="amd64";;
  *sparc*)   hosttype="sparc";;
  *sun*)     hosttype="sparc";;
  *ia64*)    hosttype="ia64";;
  *itanium*) hosttype="ia64";;
  *powerpc*) hosttype="powerpc";;
  *ppc*)     hosttype="powerpc";;
  *alpha*)   hosttype="alpha";;
  *mips*)    hosttype="mips";;
  *arm*)     hosttype="arm";;
  *)         hosttype="${HOSTTYPE}";;
esac

# Define PARSECPLAT
PARSECPLAT="${hosttype}-${ostype}.gcc"
