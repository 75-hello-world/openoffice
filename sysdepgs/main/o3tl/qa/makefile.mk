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



.IF "$(WITH_CPPUNIT)" != "YES" || "$(GUI)" == "OS2"

@all:
.IF "$(GUI)" == "OS2"
	@echo "Skipping, cppunit broken."
.ELIF "$(WITH_CPPUNIT)" != "YES"
	@echo "cppunit disabled. nothing do do."
.END

.ELSE # "$(WITH_CPPUNIT)" != "YES" || "$(GUI)" == "OS2"

PRJ=..

PRJNAME=o3tl
TARGET=tests

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

#building with stlport, but cppunit was not built with stlport
.IF "$(USE_SYSTEM_STL)"!="YES"
.IF "$(SYSTEM_CPPUNIT)"=="YES"
CFLAGSCXX+=-DADAPT_EXT_STL
.ENDIF
.ENDIF

CFLAGSCXX += $(CPPUNIT_CFLAGS)

.IF "$(L10N_framework)"==""
# --- Common ----------------------------------------------------------

# BEGIN ----------------------------------------------------------------
SHL1OBJS=  \
	$(SLO)$/cow_wrapper_clients.obj     \
	$(SLO)$/test-cow_wrapper.obj	    \
    $(SLO)$/test-vector_pool.obj	\
    $(SLO)$/test-heap_ptr.obj           \
    $(SLO)$/test-range.obj

SHL1TARGET= tests
SHL1STDLIBS= 	$(SALLIB)		 \
				$(CPPUNITLIB)

SHL1IMPLIB= i$(SHL1TARGET)

DEF1NAME    =$(SHL1TARGET)
SHL1VERSIONMAP = export.map
SHL1RPATH = NONE

# END ------------------------------------------------------------------

#------------------------------- All object files -------------------------------
# do this here, so we get right dependencies
SLOFILES=$(SHL1OBJS)

# --- Targets ------------------------------------------------------
.ENDIF 		# L10N_framework

.INCLUDE : target.mk

# --- Enable test execution in normal build ------------------------
.IF "$(L10N_framework)"==""
.INCLUDE : _cppunit.mk
.ENDIF 		# L10N_framework

.ENDIF # "$(WITH_CPPUNIT)" != "YES" || "$(GUI)" == "OS2"
