XMAKE_EXECUTABLE=xmake

.PHONY: install
install:
	${XMAKE_EXECUTABLE} require

.PHONY: clean
clean:
	rm -rf ./build/

.PHONY: build-debug
build-debug:
	${XMAKE_EXECUTABLE} config -m debug
	${XMAKE_EXECUTABLE} build

.PHONY: build-release
build-release:
	${XMAKE_EXECUTABLE} config -m release
	${XMAKE_EXECUTABLE} build

# Utility function for the C/C++ VSCode extension
.PHONY: create-compile-commands
create-compile-commands:
	${XMAKE_EXECUTABLE} project -k compile_commands .vscode/
