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



PRJ=..$/..$/..

PRJNAME=bridges
TARGET=sunpro5_uno
LIBTARGET=no
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

# disable check for PIC code as it would complain about 
# hand coded assembler
CHECKFORPIC=

.IF "$(COM)$(CPU)" == "C50I" || "$(COM)$(CPU)" == "C52I"

CFLAGS += -O5 -xO5

SLOFILES= \
	$(SLO)$/cpp2uno.obj		\
	$(SLO)$/uno2cpp.obj		\
	$(SLO)$/except.obj		\
	$(SLO)$/call.obj

SHL1TARGET= $(TARGET)

SHL1DEF=    $(MISC)$/$(SHL1TARGET).def
SHL1IMPLIB= i$(TARGET)
SHL1VERSIONMAP=..$/..$/bridge_exports.map
SHL1RPATH=  URELIB

SHL1OBJS = $(SLOFILES)
SHL1LIBS = $(SLB)$/cpp_uno_shared.lib

SHL1STDLIBS= \
	$(CPPULIB)	\
	$(SALLIB)

.ENDIF

.INCLUDE :	target.mk

$(SLO)$/%.obj: %.s
	CC -c -o $(SLO)$/$(@:b).o $< && touch $@

