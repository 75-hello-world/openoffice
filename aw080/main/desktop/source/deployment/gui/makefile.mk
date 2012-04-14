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



PRJ = ..$/..$/..

PRJNAME = desktop
TARGET = deploymentgui
ENABLE_EXCEPTIONS = TRUE
#USE_DEFFILE = TRUE
NO_BSYMBOLIC = TRUE
USE_PCH :=
ENABLE_PCH :=
PRJINC:=..$/..

.IF "$(GUI)"=="OS2"
TARGET = deplgui
.ENDIF

.INCLUDE : settings.mk
.INCLUDE : $(PRJ)$/source$/deployment$/inc$/dp_misc.mk
DLLPRE =

SLOFILES = \
        $(SLO)$/dp_gui_service.obj \
        $(SLO)$/dp_gui_extlistbox.obj \
        $(SLO)$/dp_gui_dialog2.obj \
        $(SLO)$/dp_gui_theextmgr.obj \
        $(SLO)$/license_dialog.obj \
        $(SLO)$/dp_gui_dependencydialog.obj \
        $(SLO)$/dp_gui_thread.obj \
        $(SLO)$/dp_gui_updatedialog.obj \
        $(SLO)$/dp_gui_updateinstalldialog.obj \
        $(SLO)$/dp_gui_autoscrolledit.obj \
        $(SLO)$/dp_gui_system.obj \
        $(SLO)$/dp_gui_extensioncmdqueue.obj \
        $(SLO)$/descedit.obj

.IF "$(GUI)"=="OS2"
SHL1TARGET = $(TARGET)$(DLLPOSTFIX)
.ELSE
SHL1TARGET = $(TARGET)$(DLLPOSTFIX).uno
.ENDIF
SHL1VERSIONMAP = $(SOLARENV)/src/component.map

SHL1STDLIBS = \
        $(SALLIB) \
        $(SALHELPERLIB) \
        $(CPPULIB) \
        $(CPPUHELPERLIB) \
        $(UCBHELPERLIB) \
        $(COMPHELPERLIB) \
        $(UNOTOOLSLIB) \
        $(TOOLSLIB) \
        $(I18NISOLANGLIB) \
        $(TKLIB) \
        $(VCLLIB) \
        $(SVTOOLLIB)	\
        $(SVLLIB)  \
        $(SVXLIB) \
        $(SVXCORELIB) \
        $(SFXLIB) \
        $(DEPLOYMENTMISCLIB) \
        $(OLE32LIB)

SHL1DEPN =
SHL1IMPLIB = i$(TARGET)
SHL1LIBS = $(SLB)$/$(TARGET).lib
SHL1DEF = $(MISC)$/$(SHL1TARGET).def

DEF1NAME = $(SHL1TARGET)
#DEFLIB1NAME = $(TARGET)
#DEF1DEPN =

SRS1NAME = $(TARGET)
SRC1FILES = \
        dp_gui_dialog.src \
        dp_gui_dialog2.src \
        dp_gui_backend.src \
        dp_gui_dependencydialog.src \
        dp_gui_updatedialog.src \
        dp_gui_versionboxes.src \
        dp_gui_updateinstalldialog.src

RESLIB1NAME = $(TARGET)
RESLIB1SRSFILES = $(SRS)$/$(TARGET).srs
RESLIB1IMAGES=	$(PRJ)$/res

.INCLUDE : target.mk


ALLTAR : $(MISC)/deploymentgui.component

$(MISC)/deploymentgui.component .ERRREMOVE : \
        $(SOLARENV)/bin/createcomponent.xslt deploymentgui.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt deploymentgui.component
