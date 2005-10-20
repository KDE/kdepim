if enable_indexlib_unittests
TESTDRIVER = testdriver
else
TESTDRIVER =
endif
noinst_PROGRAMS = $(TESTDRIVER)
testdriver_SOURCES = testdriver.cpp create-test.cpp ifile-test.cpp leafdatavector-test.cpp match-test.cpp mempool-test.cpp memvector-test.cpp slow-test.cpp stringarray-test.cpp stringset-test.cpp tokenizer-test.cpp
testdriver_CXXFLAGS = -I.. $(all_includes)
testdriver_LDADD = ../libindex.la -lboost_unit_test_framework -lz
