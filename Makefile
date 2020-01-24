SUBDIRS = src

include $(PROJECT_ROOT)/env/makerules/subdirs.rules

artefacts: src

help:
	@echo "usage: make [all|clean]"
	@echo "       all:   build all artefacts"
	@echo "       clean: remove all artefacts"
