all: layer1_sw_tutorial.pdf

layer1_sw_tutorial.pdf: layer1_sw_tutorial.aux
	pdflatex layer1_sw_tutorial.tex

layer1_sw_tutorial.aux: layer1_sw_tutorial.tex
	pdflatex layer1_sw_tutorial.tex

clean:
	rm -f layer1_sw_tutorial.pdf layer1_sw_tutorial.aux

.PHONY: all clean
