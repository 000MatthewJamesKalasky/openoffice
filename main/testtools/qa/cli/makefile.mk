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
nothing .PHONY:
.ELSE

PRJ := ..$/..
PRJNAME := testtools
TARGET := test_cli

.IF "$(OOO_JUNIT_JAR)" != ""
PACKAGE = clitest

# here store only Files which contain a @Test
JAVATESTFILES = \
    CLITest.java

# put here all other files
JAVAFILES = $(JAVATESTFILES)

JARFILES = ridl.jar unoil.jar jurt.jar juh.jar java_uno.jar OOoRunner.jar
EXTRAJARFILES = $(OOO_JUNIT_JAR)

EXETARGET = $(BIN)$/cli_bridgetest_inprocess.exe
EXEARG_WIN= $(BIN)$/cli_bridgetest_inprocess.ini
EXEARG= $(strip $(subst,$/,/ $(EXEARG_WIN)))

JAVAIFLAGS =\
    -Dcli_test=$(EXETARGET) \
    -Dcli_test_arg=$(EXEARG)

# Sample how to debug
# JAVAIFLAGS+=-Xdebug  -Xrunjdwp:transport=dt_socket,server=y,address=9003,suspend=y

.END

.INCLUDE: settings.mk
.INCLUDE: target.mk
.INCLUDE: installationtest.mk

ALLTAR : javatest

.END

