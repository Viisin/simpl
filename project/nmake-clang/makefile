
CC=clang
AR=llvm-ar
OBJDUMP=llvm-objdump
RM=del

WARN_OPT=-Werror -Wundef
WARN_C_OPT=-Wuninitialized -Wreturn-type -Wredundant-decls -Wimplicit-function-declaration -Wtype-limits
GENERAL_C_OPT=-fno-zero-initialized-in-bss -fno-omit-frame-pointer -fno-optimize-sibling-calls
TARGET_OPT= -msoft-float -mno-sse -m64
C_OPT= -o $@ -c -O2 -std=c99 -DNDEBUG $(GENERAL_C_OPT) $(TARGET_OPT) $(WARN_OPT) $(WARN_C_OPT)

MODULE=simpl
COMPAT_LIB_UNIT_TEST_SRC_PATH=..\..\unit-test^\
COMPAT_LIB_SRC_PATH=..\..\src^\
COMPAT_LIB_OUT_PATH=..\..\mak.clang-vs^\
COMPAT_LIB_C_OPTS=-I..\..\include

LIB_SIMPL=simpl
LIB_SIMPL_C_OPTS=$(COMPAT_LIB_C_OPTS)
LIB_SIMPL_C_OBJS=$(COMPAT_LIB_OUT_PATH)simpl.o
LIB_SIMPL_UNIT_TEST_C_OPTS=$(COMPAT_LIB_C_OPTS)
LIB_SIMPL_UNIT_TEST_C_OBJS=$(COMPAT_LIB_OUT_PATH)simpl-test-main.o

help:
	@echo [$(MODULE)] usage:
	@echo [$(MODULE)] nmake (all/clean/lib/test)

# rebuild all
all: clean lib test
	@echo [$(MODULE)] rebuild all finished!
clean:
	@if exist $(COMPAT_LIB_OUT_PATH) ($(RM) /F /Q $(COMPAT_LIB_OUT_PATH)*)
	@echo [$(MODULE)] clean all!
	@echo;

# build static library
lib: _mkdir $(COMPAT_LIB_OUT_PATH)$(MODULE).lib
	@echo [$(MODULE)] $(MODULE).lib build finished!
	@echo;

# build unit test
test: _mkdir $(COMPAT_LIB_OUT_PATH)$(LIB_SIMPL)-unit-test.exe
	@echo [$(MODULE)] $(MODULE)-unit-test.exe build finished!
	@echo;

_mkdir:
	@if not exist $(COMPAT_LIB_OUT_PATH) (mkdir $(COMPAT_LIB_OUT_PATH))

# libaray source rule
$(LIB_SIMPL_C_OBJS): $(COMPAT_LIB_SRC_PATH)$(@B).c
	@echo [$(MODULE)] compiling $(COMPAT_LIB_SRC_PATH)$(@B).c...
	@$(CC) $(LIB_SIMPL_C_OPTS) $(C_OPT) $(COMPAT_LIB_SRC_PATH)$(@B).c

# link static library object
$(COMPAT_LIB_OUT_PATH)$(MODULE).lib: $(LIB_SIMPL_C_OBJS)
	@echo [$(MODULE)] start building $(MODULE).lib...
	@$(AR) -r $@ $(LIB_SIMPL_C_OBJS)
	@echo [$(MODULE)] generating $*.objdump...
	@$(OBJDUMP) -d $@ > $*.objdump

# unit-test source rule
$(LIB_SIMPL_UNIT_TEST_C_OBJS): $(COMPAT_LIB_UNIT_TEST_SRC_PATH)$(@B).c
	@echo [$(MODULE)] compiling $(COMPAT_LIB_UNIT_TEST_SRC_PATH)$(@B).c...
	@$(CC) $(LIB_SIMPL_UNIT_TEST_C_OPTS) $(C_OPT) $(COMPAT_LIB_UNIT_TEST_SRC_PATH)$(@B).c
	@echo [$(MODULE)] generating $*.objdump...
	@$(OBJDUMP) -d $@ > $*.objdump

# build unit-test program
$(COMPAT_LIB_OUT_PATH)$(LIB_SIMPL)-unit-test.exe: $(COMPAT_LIB_OUT_PATH)$(MODULE).lib $(LIB_SIMPL_UNIT_TEST_C_OBJS)
	@echo [$(MODULE)] start building $(MODULE)-unit-test.exe...
	@$(CC) $(LIB_SIMPL_UNIT_TEST_C_OBJS) -L$(COMPAT_LIB_OUT_PATH) -l$(MODULE) -o $(COMPAT_LIB_OUT_PATH)$(LIB_SIMPL)-unit-test.exe
