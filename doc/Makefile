HTML_FILES	= build.html hacking.html licenses.html todo.html tests.html

.PHONY: all
all: $(HTML_FILES)

%.html: %.asciidoc
	asciidoc -b html -o $@ $<

.PHONY: clean
clean:
	rm -f $(HTML_FILES)
