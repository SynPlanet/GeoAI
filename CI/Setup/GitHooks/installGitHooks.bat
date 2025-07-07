@echo off

call ..\..\Config.bat

xcopy /y %G_PROJECT_PATH%\CI\Setup\GitHooks\*.? %G_PROJECT_PATH%\.git\hooks