export REPORTER = BashTapReporter

test:
	@NODE_ENV=test node test/run.js

.PHONY: test
