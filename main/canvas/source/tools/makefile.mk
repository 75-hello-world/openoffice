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



PRJ=..$/..

PRJNAME=canvas
TARGET=canvastools
ENABLE_EXCEPTIONS=TRUE
LIBTARGET=NO

# --- Settings -----------------------------------------------------------

.INCLUDE :	settings.mk

# --- Common ----------------------------------------------------------

.IF "$(verbose)"!="" || "$(VERBOSE)"!=""
CDEFS+= -DVERBOSE
.ENDIF
.IF "$(profiler)"!="" || "$(PROFILER)"!=""
CDEFS+= -DPROFILER
.ENDIF

CDEFS+= -DCANVASTOOLS_DLLIMPLEMENTATION

#CFLAGS +:= /Ox /Ot					# THIS IS IMPORTANT


.IF "$(L10N_framework)"==""

SHL1TARGET= 	$(TARGET)$(DLLPOSTFIX)
SHL1IMPLIB= 	i$(TARGET)
SHL1STDLIBS=	$(SALLIB) $(CPPULIB) $(BASEGFXLIB) $(CPPUHELPERLIB) $(COMPHELPERLIB) $(VCLLIB) $(TKLIB) $(TOOLSLIB)
.IF "$(GUI)" == "WNT"
SHL1STDLIBS += $(WINMMLIB) $(KERNEL32LIB)
.ENDIF
SHL1USE_EXPORTS=name
SHL1DEF=	$(MISC)$/$(SHL1TARGET).def
DEF1NAME	=$(SHL1TARGET)

SLOFILES =	\
	$(SLO)$/cachedprimitivebase.obj \
	$(SLO)$/canvascustomspritehelper.obj \
	$(SLO)$/canvastools.obj \
	$(SLO)$/elapsedtime.obj \
	$(SLO)$/parametricpolypolygon.obj \
	$(SLO)$/prioritybooster.obj \
	$(SLO)$/propertysethelper.obj \
	$(SLO)$/spriteredrawmanager.obj \
	$(SLO)$/surface.obj \
	$(SLO)$/surfaceproxy.obj \
	$(SLO)$/surfaceproxymanager.obj \
	$(SLO)$/pagemanager.obj \
	$(SLO)$/page.obj \
	$(SLO)$/verifyinput.obj

SHL1OBJS=	$(SLOFILES)

.ENDIF

# ==========================================================================

.INCLUDE :	target.mk

