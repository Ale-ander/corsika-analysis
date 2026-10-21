##############################################################################################
# export coast dir
#--------------------------------------------------------------------------------------------

export COAST_DIR=../

#--------------------------------------------------------------------------------------------

SRC_DIR   = src
BUILD_DIR = build

LDFLAGS   = ${CXXFLAGS} -fPIC -ggdb3
LDFLAGS  += $(shell root-config --libs)
LDFLAGS  += -L$(COAST_DIR)/lib -L$(COAST_DIR)/lib/unknown
LDFLAGS  += -lCorsikaFileIO
LDFLAGS  += -lCorsikaIntern

CPPFLAGS  = ${CXXFLAGS} -fPIC -ggdb3
CPPFLAGS += -I$(COAST_DIR)/include
CPPFLAGS += -I$(SRC_DIR)
CPPFLAGS += $(shell root-config --cflags)
# genera i file .d con le dipendenze dagli header
CPPFLAGS += -MMD -MP

EXE       = energySpectra
EXE      += angularDistributionAzimuth
EXE      += angularDistributionZenith
EXE      += groundMomenta
EXE      += footprint
EXE      += dummySim
EXE      += longReader

TARGETS   = $(addprefix $(BUILD_DIR)/, ${EXE})

all: ${TARGETS}

#--------------------------------------------------------------------------------------------
# regola generica: src/%.cc -> build/%.o
#--------------------------------------------------------------------------------------------

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cc | $(BUILD_DIR)
	${CXX} ${CPPFLAGS} -c $< -o $@

$(BUILD_DIR):
	mkdir -p $@

#--------------------------------------------------------------------------------------------
# eseguibili
#--------------------------------------------------------------------------------------------

$(BUILD_DIR)/energySpectra: $(BUILD_DIR)/energySpectra.o
	${CXX} $^ ${LDFLAGS} -o $@

$(BUILD_DIR)/angularDistributionZenith: $(BUILD_DIR)/angularDistributionZenith.o
	${CXX} $^ ${LDFLAGS} -o $@

$(BUILD_DIR)/angularDistributionAzimuth: $(BUILD_DIR)/angularDistributionAzimuth.o
	${CXX} $^ ${LDFLAGS} -o $@

$(BUILD_DIR)/groundMomenta: $(BUILD_DIR)/groundMomenta.o
	${CXX} $^ ${LDFLAGS} -o $@

$(BUILD_DIR)/footprint: $(BUILD_DIR)/footprint.o
	${CXX} $^ ${LDFLAGS} -o $@

$(BUILD_DIR)/dummySim: $(BUILD_DIR)/dummySim.o $(BUILD_DIR)/dummyExperiment.o
	${CXX} $^ ${LDFLAGS} -o $@

$(BUILD_DIR)/longReader: $(BUILD_DIR)/longReader.o
	${CXX} $^ ${LDFLAGS} -o $@

#--------------------------------------------------------------------------------------------

clean:
	@rm -rf $(BUILD_DIR)

# includi le dipendenze generate (se esistono)
-include $(wildcard $(BUILD_DIR)/*.d)

.PHONY: all clean