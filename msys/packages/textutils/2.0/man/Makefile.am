## Process this file with automake to produce Makefile.in -*-Makefile-*-
man_MANS = cat.1 cksum.1 comm.1 csplit.1 cut.1 expand.1 fmt.1 fold.1 head.1 \
join.1 md5sum.1 nl.1 od.1 paste.1 pr.1 ptx.1 sort.1 split.1 sum.1 tac.1 tail.1 \
tr.1 tsort.1 unexpand.1 uniq.1 wc.1
man_aux = $(man_MANS:.1=.x)

EXTRA_DIST = $(man_MANS)

EXTRA_DIST = $(man_MANS) $(HELP2MAN) $(man_aux) \
  Makefile.maint GNUmakefile Makefile.summ
PERL = @PERL@
HELP2MAN = help2man
MAINTAINERCLEANFILES = $(man_MANS)

# For additional rules usually of interest only to the maintainer,
# see GNUmakefile and Makefile.maint.
