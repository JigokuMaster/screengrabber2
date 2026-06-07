APP_NAME=ScreenGrabber2
SIS="$(APP_NAME)_gcce.sis"
SIS_LITE="$(APP_NAME)_lite.sis"
EXE=$(APP_NAME).exe
EXE_FP=$(EPOCROOT)/epoc32/release/gcce/urel/$(EXE)


prebuild:
	cd group && bldmake bldfiles gcce urel

build:
	cd group && abld build -v gcce urel

clean: 
	cd group && abld reallyclean gcce urel


mksis:
	cp -v data/tmp/about.html data
	cp -v data/tmp/$(APP_NAME)_gcce.pkg sis

	cd sis && PLATFORM=gcce TARGET=urel makesis -v -d$(EPOCROOT) $(APP_NAME)_gcce.pkg

mksis_lite:
	cp -v data/tmp/about_lite.html data/about.html
	cp -v data/tmp/$(APP_NAME)_lite.pkg sis/
	cd sis && PLATFORM=gcce TARGET=urel makesis -v -d$(EPOCROOT) $(APP_NAME)_lite.pkg

depoly:
	renv send "sis/$(SIS)" "E:\\$(SIS)"
	renv send "sis/$(SIS_LITE)" "E:\\$(SIS_LITE)"

run:
	renv send "$(EXE_FP)" "C:\\sys\\bin\\$(EXE)"
	renv start -w $(EXE)

