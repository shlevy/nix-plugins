.POSIX:

.PHONY: all

all: Makefile

UNAME=uname
SED=sed

Makefile: Makefile.in
	if [ "$$($(UNAME))" = "Darwin" ]; then \
	  undefined="-undefined dynamic_lookup"; \
	else \
	  undefined=""; \
	fi; \
	$(SED) "s/@undefined@/$$undefined/" < $< > $@

