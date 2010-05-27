@echo off
SkinTranslate.exe default-en.xml default-en.pot
for %%i in (*.po) do msgmerge.exe --no-escape --quiet --no-wrap --update --backup=off "%%i" default-en.pot
for %%i in (*.po) do SkinTranslate.exe default-en.xml "%%i" #.xml