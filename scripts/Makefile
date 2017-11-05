# Test the comment extractor
.PHONY: all
.SUFFIXES: .py

PYTHOH=/cygdrive/c/bin/python/python
TARGET_SCRIPT=comment_extractor/comment_extractor.py
TEST_SCRIPT=tests/test_comment_extractor.py

all:
	$(PYTHOH) -m unittest discover tests
	bash -c "diff --strip-trailing-cr <($(PYTHOH) comment_extractor/comment_extractor.py data/input.txt) data/expected.txt"
	$(PYTHOH) $(TARGET_SCRIPT) $(TARGET_SCRIPT)
	$(PYTHOH) $(TARGET_SCRIPT) $(TEST_SCRIPT)
