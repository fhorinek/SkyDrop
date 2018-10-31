all: Manual-of-SkyDrop.pdf

clean:
	rm -f Manual-of-SkyDrop.html Manual-of-SkyDrop.pdf

distclean: clean
	rm -f Manual-of-SkyDrop.md

Manual-of-SkyDrop.md:
	wget --quiet https://github.com/fhorinek/SkyDrop/wiki/Manual-of-SkyDrop.md

Manual-of-SkyDrop.html: Manual-of-SkyDrop.md
	pandoc Manual-of-SkyDrop.md -o Manual-of-SkyDrop.html

Manual-of-SkyDrop.pdf: Manual-of-SkyDrop.html
	wkhtmltopdf --page-size a5 Manual-of-SkyDrop.html Manual-of-SkyDrop.pdf

