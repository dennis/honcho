bin_PROGRAMS = honcho

honcho_SOURCES = main.c \
	cmd_cat.c cmd_cat.h \
	cmd_execute.c cmd_execute.h \
	cmd_state.c cmd_state.h \
	cmd_status_overview.c cmd_status_overview.h \
	cmd_status_query.c cmd_status_query.h \
	cmd_path.c cmd_path.h \
	utils.c utils.h 

DEFS = @DEFS@ -DDATA_DIR=\"$(data_dir)\"

install-exec-hook:
	-if test ! -d $(DESTDIR)/$(data_dir); \
		$(mkinstalldirs) $(DESTDIR)/$(data_dir)/queue/default; \
	fi
