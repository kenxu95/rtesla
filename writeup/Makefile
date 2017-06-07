default: main.pdf

texfiles := $(wildcard *.tex)

main.pdf: $(texfiles) touch
	pdflatex main
	bibtex main
	pdflatex main
	@pdflatex main < /dev/null > /dev/null || true
	@grep -A 4 "Emergency stop" main.log || grep Warning main.log || true
	@if grep 'Rerun to get ' main.log ; then echo "Rerunning pdflatex" && pdflatex main < /dev/null > /dev/null; fi
	open main.pdf

touch:
	touch main.pdf

clean:
	rm -rf main.aux main.log main.out main.bbl main.blg main.pdf
