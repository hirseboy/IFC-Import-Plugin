@echo off

goto check_permissions

:create_symlinks
	set lib_list=IBK QtExt IBKMK clipper TiCPP Nandrad Vicus CCM DataIO glm VicOSM QuaZIP HiGHS NandradFMUGenerator

	(for %%a in (%lib_list%) do (
	   echo -----------------------------
	   IF NOT EXIST "%~dp0..\SIM-VICUS\externals\%%a" (
		echo Verzeichnis '..\SIM-VICUS\externals\%%a' existiert nicht. Bitte submodules in git-repository pruefen.
		exit /b 1
	   )
	   if exist %~dp0\%%a (
			echo Loesche vorhandenen Symlink der Bibliothek '%%a'
			rmdir %~dp0\%%a
	   )
	   echo Erzeuge Symlink fuer Bibliothek '%%a'
	   mklink /d %~dp0\%%a %~dp0..\SIM-VICUS\externals\%%a
	))

exit /b 0

:check_permissions
    echo Adminrechte sind erforderlich!

    net session >nul 2>&1
    if %errorLevel% == 0 (
        echo Adminrechte wurden gewaehrt.
		goto create_symlinks
    ) else (
        echo Keine Adminrechte gefunden. Bitte erneut mit Adminrechten ausfuehren.
		exit /b 1
    )
