The MS Access 2000/2003 database located here serves for the purpose of maintaining and testing schemas.
This database can be used by translators to test their translations.

About using Access. It is very simple. Just open Shareaza_schemas.mdb file and you will find window with two buttons: one for entering your translation and the other for generating schemas files.
1. Press "Create Language/Edit Data".
2. Press "Create Language".
3. Enter, for example, "en" for Language code, "English" for Language and your name or nick for Author.
4. Press OK.
5. Select your language from dropdown list.
6. Press "Datasheet View" if you want to use a spreadsheet like in Excel and enter your translation.
7. Do not press "Datasheet View" if you want to enter data record by record. Go to the next record by pressing navigation buttons at the bottom of window.
8. Close "DataEntry" form.
9. Press "Generate/Edit Schemas".
10. Press "Edit Languages" if you want to disable or enable any language.
11. Press "Generate all files".
12. Go to the folder where you saved Shareaza_schemas.mdb file and you will find Shareaza subfolder. Here you will find all files that you need to copy to Schemas folder.

There is one caveat for advanced users. The database changes the default alphabet sorting order to "General" (i.e. English). This is needed to correctly sort english words ("Y" at the end and not after "I"). If you want to compact the database then do not change this value from Tools->Options->General->New Database Sort Order when you compact it. When the sorting order is English you can paste to/from Excel from/to Datasheet view. Otherwise, you will have a mess.

Note: In East Asian Windows you may need to disable some languages to test schemas, otherwise auto-generation of XML files may not work. The required texts are always english and your translation, nothing else is mandatory.

Notes for developers: If you need to update a language translation, never overwrite the database file with the file provided by translators.
The best way to do updates is to re-link the table TextData1, open in and copy-paste data of the appropriate language to TextData table.
The prebuilt queries can be used to merge data too, but they have to be modified before. The only thing is needed--to change language code.
"Files Changed" query will tell what files were updated in that case and you will be sure what files have to be committed to SVN repository.