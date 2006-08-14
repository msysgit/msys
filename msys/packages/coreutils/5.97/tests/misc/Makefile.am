## Process this file with automake to produce Makefile.in -*-Makefile-*-.

EXTRA_DIST = $(TESTS)

TESTS_ENVIRONMENT = \
  top_srcdir=$(top_srcdir) \
  srcdir=$(srcdir) \
  PACKAGE_VERSION=$(PACKAGE_VERSION) \
  PERL="$(PERL)" \
  BUILD_SRC_DIR="`pwd`/../../src" \
  PATH="`pwd`/../../src$(PATH_SEPARATOR)$$PATH" \
  PROG=`../../src/basename -- "$$tst"`

# Do not choose a name that is a shell keyword like 'if', or a
# commonly-used utility like 'cat' or 'test', as the name of a test.
# Otherwise, VPATH builds will fail on hosts like Solaris, since they
# will expand 'if test ...' to 'if .../test ...', and the '.../test'
# will execute the test script rather than the standard utility.

TESTS = \
  cat-proc \
  stat-printf \
  stat-fmt \
  date \
  tac-continue \
  close-stdout \
  pwd-long \
  date-sec \
  paste-no-nl \
  expand \
  fold \
  nohup \
  head-elide-tail \
  split-fail \
  false-status \
  tty-eof \
  printf-hex \
  nl split-l printf split-a head-pos sort-merge head-c csplit nice pathchk1
