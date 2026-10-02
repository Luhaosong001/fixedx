CC      = gcc
CFLAGS  = -std=c99 -Wall -Wextra -Werror -O2 -Iinclude
BUILD   = build

.PHONY: test clean

test: $(BUILD)/test_fixedx
	./$(BUILD)/test_fixedx

$(BUILD)/test_fixedx: tests/test_fixedx.c src/fixedx.c src/sintab.inc include/fixedx/fixedx.h
	@mkdir -p $(BUILD)
	$(CC) $(CFLAGS) tests/test_fixedx.c src/fixedx.c -o $@ -lm

clean:
	rm -rf $(BUILD)
