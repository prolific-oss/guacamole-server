#
# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.
#
# NOTE: Parts of this file (Makefile.am) are automatically transcluded verbatim
# into Makefile.in. Though the build system (GNU Autotools) automatically adds
# its own license boilerplate to the generated Makefile.in, that boilerplate
# does not apply to the transcluded portions of Makefile.am which are licensed
# to you by the ASF under the Apache License, Version 2.0, as described above.
#

AUTOMAKE_OPTIONS = foreign

AM_CPPFLAGS = -include config.h
ACLOCAL_AMFLAGS = -I m4

check_PROGRAMS = test_guacenc
TESTS = $(check_PROGRAMS)

test_guacenc_SOURCES =          \
    ../video-timeline.c         \
    video-timeline/frame-index.c

test_guacenc_CFLAGS =               \
    -Werror -Wall -pedantic         \
    -I$(top_srcdir)/src/guacenc     \
    @LIBGUAC_INCLUDE@

test_guacenc_LDADD = \
    @CUNIT_LIBS@

GEN_RUNNER = $(top_srcdir)/util/generate-test-runner.pl
CLEANFILES = _generated_runner.c

_generated_runner.c: $(test_guacenc_SOURCES)
	$(AM_V_GEN) $(GEN_RUNNER) $(test_guacenc_SOURCES) > $@

nodist_test_guacenc_SOURCES = \
    _generated_runner.c

LOG_DRIVER =                \
    env AM_TAP_AWK='$(AWK)' \
    $(SHELL) $(top_srcdir)/build-aux/tap-driver.sh
