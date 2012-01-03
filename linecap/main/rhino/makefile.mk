#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

PRJ=.

PRJNAME=ooo_rhino
TARGET=ooo_rhino

.IF "$(SOLAR_JAVA)"==""
all:
        @echo java disabled

.ELIF "$(ENABLE_JAVASCRIPT)"!="YES"
all:
        @echo javascript support disabled
.ELSE

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk
.INCLUDE :  antsettings.mk

# --- Files --------------------------------------------------------

#.IF "$(DISABLE_RHINO)" == ""

TARFILE_NAME=rhino1_7R3
TARFILE_MD5=99d94103662a8d0b571e247a77432ac5
TARFILE_ROOTDIR=rhino1_7R3

ADDITIONAL_FILES= \
	toolsrc/org/mozilla/javascript/tools/debugger/OfficeScriptInfo.java

PATCH_FILES=rhino1_7R3.patch

.IF "$(JAVACISGCJ)"=="yes"
JAVA_HOME=
.EXPORT : JAVA_HOME
BUILD_ACTION=$(ANT) -Dbuild.label="build-$(RSCREVISION)" -Dbuild.compiler=gcj -Dno-xmlbeans=true jar
.ELSE
BUILD_ACTION=$(ANT) -Dbuild.label="build-$(RSCREVISION)" -Dno-xmlbeans=true jar
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk

.ENDIF
