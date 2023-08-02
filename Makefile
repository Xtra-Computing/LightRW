

include mk/phase/phase_init.mk


ifndef app
build_app=${test}
APP_PATH?=app/test
else
build_app=${app}
APP_PATH?=app
endif

test?=rn_test_ddr

include mk/phase/phase_app.mk

include mk/phase/phase_misc.mk

include mk/phase/phase_sw.mk

include mk/phase/phase_hw.mk






