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
PRJNAME=sd
TARGET=filter
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Common ----------------------------------------------------------

.IF "$(dbg_anim_log)"!="" || "$(DBG_ANIM_LOG)"!=""
CDEFS+= -DDBG_ANIM_LOG
.ENDIF

# --- Files --------------------------------------------------------

SLOFILES =\
		$(SLO)$/sdfilter.obj	\
		$(SLO)$/sdpptwrp.obj	

EXCEPTIONSFILES= \
		   $(SLO)$/pptin.obj		\
		   $(SLO)$/sdfilter.obj

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk
