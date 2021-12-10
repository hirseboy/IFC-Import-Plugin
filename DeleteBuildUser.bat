forfiles /P g:\Programming\libs\IFC2BESplusplus\ /M build-* /s /C "cmd /c if @isdir==TRUE rmdir /s /q @file"
del /S /Q *.pro.user build-*
PAUSE
