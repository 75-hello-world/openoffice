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



PRJ := ..$/..
PRJNAME := codemaker
TARGET := test_codemaker_cppumaker

ENABLE_EXCEPTIONS := TRUE

.INCLUDE: settings.mk

.IF "$(ENABLE_UNIT_TESTS)" != "YES"
all:
	@echo unit tests are disabled. Nothing to do.

.ELSE

INCPRE += $(MISC)$/$(TARGET)$/inc

APP1TARGET = $(TARGET)
APP1OBJS = $(SLO)$/test_codemaker_cppumaker.obj
APP1STDLIBS = $(CPPULIB) $(GTESTLIB) $(SALLIB) $(TESTSHL2LIB)
APP1VERSIONMAP = version.map
APP1IMPLIB = i$(APP1TARGET)
DEF1NAME = $(APP1TARGET)
APP1TEST = enabled

SLOFILES = $(APP1OBJS)

.INCLUDE: target.mk

$(APP1OBJS): $(MISC)$/$(TARGET).cppumaker.flag

$(MISC)$/$(TARGET).cppumaker.flag: $(BIN)$/cppumaker$(EXECPOST)
$(MISC)$/$(TARGET).cppumaker.flag: $(MISC)$/$(TARGET).rdb
    - $(MKDIRHIER) $(MISC)$/$(TARGET)$/inc
    $(AUGMENT_LIBRARY_PATH) $(BIN)$/cppumaker$(EXECPOST) \
        -O$(MISC)$/$(TARGET)$/inc -BUCR -C $< $(SOLARBINDIR)$/udkapi.rdb
    $(TOUCH) $@

$(MISC)$/$(TARGET).rdb: $(MISC)$/$(TARGET)$/types.urd
    - rm $@
    $(REGMERGE) $@ /UCR $<

$(MISC)$/$(TARGET)$/types.urd: types.idl
    - $(MKDIR) $(MISC)$/$(TARGET)
    $(IDLC) -O$(MISC)$/$(TARGET) -I$(SOLARIDLDIR) -cid -we $<


.ENDIF # "$(ENABLE_UNIT_TESTS)" != "YES"
