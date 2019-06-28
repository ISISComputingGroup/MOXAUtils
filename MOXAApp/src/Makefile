TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#==================================================
# build a support library

PROD_HOST += moxautil

IPSERIALROOT = $(ICPBINARYDIR)/MOXA/ipserial

moxautil_CXXFLAGS += -I$(CLI11) -I$(IPSERIALROOT)/include

ifneq ($(findstring windows,$(EPICS_HOST_ARCH)),)
IPSerial_DIR = $(IPSERIALROOT)/lib/x64
else
IPSerial_DIR = $(IPSERIALROOT)/lib/x86
endif

moxautil_SRCS += moxautil.cpp

moxautil_LIBS += IPSerial
moxautil_LIBS += $(EPICS_BASE_IOC_LIBS)

BIN_INSTALLS_WIN32 += $(IPSerial_DIR)/IPSerial.dll

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

