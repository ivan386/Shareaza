@echo off
for %%i in (*.po) do msgmerge.exe --no-fuzzy-matching --no-escape --quiet --no-wrap --update --backup=off "%%i" default-en.pot