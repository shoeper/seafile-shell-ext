@echo off

title �Զ�����ͼ�껺��

taskkill /f /im explorer.exe

choice /t 3 /d y /n >nul

del /f /s /q "%userprofile%\AppData\Local\Microsoft\Windows\Explorer\*.tmp"

rename "%userprofile%\AppData\Local\Microsoft\Windows\Explorer\*.db" *.db.tmp

choice /t 3 /d y /n >nul

del /f /s /q "%userprofile%\AppData\Local\Microsoft\Windows\Explorer\*.tmp"

explorer
