all: SkyDrop_user_manual_en.pdf

SkyDrop_user_manual_en.html: SkyDrop_user_manual_en.md
	pandoc SkyDrop_user_manual_en.md -o SkyDrop_user_manual_en.html 

SkyDrop_user_manual_en.pdf: SkyDrop_user_manual_en.html
	wkhtmltopdf --page-size a5 SkyDrop_user_manual_en.html SkyDrop_user_manual_en.pdf

