

app-out.xml: app.xml foo.xml process.xsl bar.xml baz.xml
	java -jar contrib/saxon9he.jar $< process.xsl > $@

app-out.xml: | contrib/saxon9he.jar


contrib/saxon9he.jar: contrib/SaxonHE9-6-0-5J.zip
	( cd contrib ; unzip -x SaxonHE9-6-0-5J.zip saxon9he.jar )
	touch $@

contrib/SaxonHE9-6-0-5J.zip:
	mkdir -p contrib
	wget --verbose http://vorboss.dl.sourceforge.net/project/saxon/Saxon-HE/9.6/SaxonHE9-6-0-5J.zip -O contrib/SaxonHE9-6-0-5J.zip