#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



.IF "$(OOO_SUBSEQUENT_TESTS)" == ""
.INCLUDE :  target.mk
.ELSE

PRJ=..$/..$/..

PRJNAME=sal
TARGET=qa_osl_profile

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

CFLAGS+= $(LFS_CFLAGS)
CXXFLAGS+= $(LFS_CFLAGS)

CFLAGSCXX += $(CPPUNIT_CFLAGS)

# BEGIN ----------------------------------------------------------------
SHL1OBJS=  \
	$(SLO)$/osl_old_testprofile.obj

SHL1TARGET= osl_old_testprofile
SHL1STDLIBS= $(SALLIB) $(CPPUNITLIB)

SHL1IMPLIB= i$(SHL1TARGET)
DEF1NAME    =$(SHL1TARGET)
SHL1VERSIONMAP = $(PRJ)$/qa$/export.map
SHL1RPATH = NONE
# END ------------------------------------------------------------------


#------------------------------- All object files -------------------------------
# do this here, so we get right dependencies
SLOFILES=\
	$(SHL1OBJS)

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE : _cppunit.mk

.END
