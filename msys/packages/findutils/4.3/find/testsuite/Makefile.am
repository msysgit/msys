AUTOMAKE_OPTIONS=dejagnu
FIND = ../find
FINDFLAGS =

DEJATOOL = find

EXTRA_DIST_XO = \
find.gnu/access.xo \
find.gnu/comma.xo \
find.gnu/delete.xo \
find.gnu/depth.xo \
find.gnu/depth-d.xo \
find.gnu/empty.xo \
find.gnu/execdir-one.xo   \
find.gnu/execdir-root-only.xo   \
find.gnu/exec-many-rtn-failure.xo   \
find.gnu/exec-many-rtn-success.xo   \
find.gnu/exec-one-rtn-fail.xo       \
find.gnu/exec-one-rtn-success.xo \
find.gnu/false.xo \
find.gnu/follow-basic.xo \
find.gnu/gnuand.xo \
find.gnu/gnunot.xo \
find.gnu/gnu-or.xo \
find.gnu/ilname.xo \
find.gnu/iname.xo \
find.gnu/inum.xo \
find.gnu/ipath.xo \
find.gnu/iregex1.xo \
find.gnu/iwholename.xo \
find.gnu/lname.xo \
find.gnu/name-opt.xo \
find.gnu/name-period.xo \
find.gnu/path.xo \
find.gnu/perm.xo \
find.gnu/perm-slash.xo \
find.gnu/posix-dflt.xo \
find.gnu/posix-h.xo \
find.gnu/posix-l.xo \
find.gnu/printf-symlink.xo \
find.gnu/printf-h.xo \
find.gnu/printf.xo \
find.gnu/print0.xo \
find.gnu/prune-default-print.xo  \
find.gnu/regex1.xo \
find.gnu/regex2.xo \
find.gnu/samefile-copy.xo \
find.gnu/samefile-link.xo \
find.gnu/samefile-same.xo \
find.gnu/samefile-symlink.xo \
find.gnu/true.xo \
find.gnu/wholename.xo \
find.gnu/xtype-symlink.xo \
find.gnu/quit.xo \
find.gnu/xtype.xo \
find.posix/links.xo \
find.posix/sv-bug-11175.xo \
find.posix/sv-bug-12181.xo \
find.posix/depth1.xo \
find.posix/sizes.xo \
find.posix/name.xo \
find.posix/perm-vanilla.xo \
find.posix/prune.xo \
find.posix/posixnot.xo \
find.posix/grouping.xo \
find.posix/and.xo \
find.posix/exec-one.xo \
find.gnu/follow-arg-parent-symlink.xo

EXTRA_DIST_EXP = \
config/unix.exp \
find.gnu/access.exp \
find.gnu/comma.exp \
find.gnu/delete.exp \
find.gnu/depth.exp \
find.gnu/depth-d.exp \
find.gnu/empty.exp \
find.gnu/execdir-one.exp \
find.gnu/execdir-root-only.exp \
find.gnu/exec-many-rtn-failure.exp  \
find.gnu/exec-many-rtn-success.exp  \
find.gnu/exec-one-rtn-fail.exp      \
find.gnu/exec-one-rtn-success.exp   \
find.gnu/follow-basic.exp \
find.gnu/false.exp \
find.gnu/gnuand.exp \
find.gnu/gnunot.exp \
find.gnu/gnu-or.exp \
find.gnu/ilname.exp \
find.gnu/iname.exp \
find.gnu/inum.exp \
find.gnu/ipath.exp \
find.gnu/iregex1.exp \
find.gnu/iwholename.exp \
find.gnu/lname.exp \
find.gnu/name-opt.exp \
find.gnu/name-period.exp \
find.gnu/path.exp \
find.gnu/print0.exp \
find.gnu/perm.exp \
find.gnu/perm-slash.exp \
find.gnu/posix-dflt.exp \
find.gnu/posix-h.exp \
find.gnu/posix-l.exp \
find.gnu/printf.exp \
find.gnu/printf-symlink.exp \
find.gnu/printf-h.exp \
find.gnu/prune-default-print.exp \
find.gnu/regex1.exp \
find.gnu/regex2.exp \
find.gnu/samefile-copy.exp \
find.gnu/samefile-link.exp \
find.gnu/samefile-same.exp \
find.gnu/samefile-symlink.exp \
find.gnu/true.exp \
find.gnu/wholename.exp \
find.gnu/xtype-symlink.exp \
find.gnu/sv-bug-12230.exp \
find.gnu/quit.exp \
find.gnu/xtype.exp \
find.posix/links.exp \
find.posix/sv-bug-11175.exp \
find.posix/sv-bug-12181.exp \
find.posix/depth1.exp \
find.posix/sizes.exp \
find.posix/name.exp \
find.posix/perm-vanilla.exp \
find.posix/posixnot.exp \
find.posix/prune.exp \
find.posix/grouping.exp \
find.posix/and.exp \
find.posix/exec-one.exp \
find.gnu/follow-arg-parent-symlink.exp

EXTRA_DIST = $(EXTRA_DIST_EXP) $(EXTRA_DIST_XO)

CLEANFILES = *.log *.sum site.exp site.bak

#DIST_SUBDIRS = config


checklists:
	cd $(srcdir) && \
	( find config find.gnu find.posix \( -name "*.exp" -o -name "*.xo" \) -print ; \
	  ls $(EXTRA_DIST_XO)  ; \
	  ls  $(EXTRA_DIST_EXP)  ; ) | \
	sort | uniq -c | awk '$$1 != 2 { print; }'
