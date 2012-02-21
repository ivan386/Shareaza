@echo off
for %%i in (*.po) do msgmerge.exe --no-escape --quiet --no-wrap --update --backup=off --no-fuzzy-matching "%%i" default-en.pot