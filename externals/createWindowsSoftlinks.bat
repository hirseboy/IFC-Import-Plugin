@echo off

goto check_permissions

:create_symlinks
	set lib_list=IBK lib_x64 QtExt IBKMK clipper TiCPP

	(for %%a in (%lib_list%) do (
	   echo -----------------------------
	   IF NOT EXIST "%~dp0..\SIM-VICUS\externals\%%a" (
		echo Verzeichnis '..\SIM-VICUS\externals\%%a' existiert nicht. Bitte submodules in git-repository prfen.
		pause
		exit /b 1
	   ) 
	   if exist %~dp0\%%a (
			echo L”sche vorhandenen Symlnk der Bibliothek '%%a'
			rmdir %~dp0\%%a	
	   )
	   echo Erzeuge Symlink fr Bibliothek '%%a'
	   mklink /d %~dp0\%%a %~dp0..\SIM-VICUS\externals\%%a
	))

pause
exit /b 1

@echo off
goto check_Permissions

:check_permissions
    echo Adminrechte sind erforderlich! 
    
    net session >nul 2>&1
    if %errorLevel% == 0 (
        echo Adminrechte wurden gew„hrt.
		goto create_symlinks
    ) else (
        echo Keine Adminrechte gefunden. Bitte erneut mit Adminrechten ausfhren.
		pause 
		exit /b 1
    )
    
    pause >nul