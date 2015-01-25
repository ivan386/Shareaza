Skin Translate Utility

This utility allow to use poEdit as translator's tool for Shareaza XML skin/language files.

==== Usage ====

1. Creating .pot-file from default English xml-file:

	SkinTranslate.exe default-en.xml default-en.pot

2. Importing existing translation from xml-file to .po-file:

	SkinTranslate.exe default-en.xml default-XX.xml default-XX.po

3. Creating translation .xml-file from .po-file:

	SkinTranslate.exe default-en.xml default-XX.po default-XX.xml

In third parameter you can use a "#" meta symbol to specify file name from second parameter, i.e. "SkinTranslate.exe default-en.xml default-XX.po #.xml"

==== Translation Guide ====

PoEdit utility can be downloaded here: http://www.poedit.net/

=== How to update existing translation ===

1. Create default-en.pot from default-en.xml (see Usage #1 above) if default-en.pot not existed or default-en.xml was updated.
2. Convert existing default-XX.xml to default-XX.po (see Usage #2 above) if default-XX.po not existed yet.
3. Run Poedit.
4. Open default-XX.po and then select menu: Catalog -> Update from POT file -> Select default-en.pot.
5. Translate new strings.
6. Convert default-XX.po back to default-XX.xml (see Usage #3 above). Check for errors.
7. Upload updated files to Translation forum.

=== How to create new translation ===

1. Create default-en.pot from default-en.xml (see Usage #1 above).
2. Run Poedit.
3. Select menu: File -> New catalog from POT file... -> Select default-en.pot.
4. Fill translation properties (codepage MUST be UTF-8). Press OK.
5. Save as default-XX.po, where XX is a two letter language code.
6. Translate.
7. Convert default-XX.po back to default-XX.xml (see Usage #3 above).
8. Make language flag default-XX.ico file.
9. Upload all files to Translation forum.

==== Contacts ====

Contact Translate Shareaza Forum http://shareaza.sourceforge.net/phpbb/viewforum.php?f=6 for report bugs, missed translations or suggestions.
