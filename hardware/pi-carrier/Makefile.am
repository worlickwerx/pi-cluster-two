if HAVE_KICAD
AM_TESTS_ENVIRONMENT = \
    export DRC_INPUT=$(abs_top_srcdir)/pi-carrier/eurocard.kicad_pcb; \
    export ERC_INPUT=$(abs_top_srcdir)/pi-carrier/eurocard.kicad_sch;

TESTS = $(abs_top_srcdir)/scripts/run_erc.sh \
	$(abs_top_srcdir)/scripts/run_drc.sh

all-local: cam.zip
endif

cam.zip: eurocard.kicad_pcb
	$(abs_top_srcdir)/scripts/plot.sh $< $@

clean-local:
	$(RM) -f cam.zip
	$(RM) -f *.rpt
